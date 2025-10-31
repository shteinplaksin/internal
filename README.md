# BlueStacks OpenGL Overlay DLL - Техническая документация

## Содержание
- [Обзор](#обзор)
- [Архитектура](#архитектура)
- [Принцип работы](#принцип-работы)
- [Компоненты системы](#компоненты-системы)
- [Технические решения](#технические-решения)
- [Процесс инжекта и инициализации](#процесс-инжекта-и-инициализации)

---

## Обзор

Данная DLL представляет собой внутриигровой оверлей для приложений, использующих OpenGL (в частности, BlueStacks Android-эмулятора). Проект реализует внедрение пользовательского графического интерфейса через перехват OpenGL API с применением техник стелс-инжекта и обфускации.

### Назначение
- Рендеринг настраиваемого GUI поверх целевого приложения
- Демонстрация техник низкоуровневого перехвата Windows API и OpenGL
- Реализация стелс-методов для скрытия присутствия DLL от детекторов

---

## Архитектура

### Основные модули

```
DLL Entry Point (DllMain)
    ↓
Worker Thread (MainThread)
    ↓
    ├─→ Stealth Module (антидетект)
    ↓
    ├─→ Hook Module (перехват wglSwapBuffers)
    ↓
    └─→ GUI Module (ImGui рендеринг)
```

### Технологический стек
- **MinHook**: библиотека для API-хукинга
- **ImGui**: библиотека для создания GUI
- **LazyImporter**: обфускация импортов WinAPI
- **xorstr**: обфускация строк в compile-time

---

## Принцип работы

### 1. Точка входа (DllMain)

При инжекте DLL в целевой процесс вызывается `DllMain` с событием `DLL_PROCESS_ATTACH`.

**Почему используется отдельный поток?**
- DLL не может выполнять длительные операции в `DllMain` - это может привести к дедлокам в загрузчике Windows
- Создание отдельного потока позволяет немедленно вернуть управление и избежать таймаутов
- Windows накладывает ограничения на вызовы API в контексте `DllMain`

```cpp
HANDLE thread_handle = CreateThread(nullptr, 0, MainThread, instance, 0, nullptr);
```

**Важно**: Используется нативный `CreateThread` вместо `std::thread` для полного контроля над потоком и совместимости с `FreeLibraryAndExitThread`.

### 2. Stealth-модуль

Реализует три техники сокрытия DLL:

#### 2.1. Проверка отладчика (`CheckDebugger`)

```cpp
if (IsDebuggerPresent()) return;
CheckRemoteDebuggerPresent(GetCurrentProcess(), &bRemoteDebugger);
```

**Зачем?** Если обнаружен отладчик, стелс-операции не выполняются, так как:
- В режиме отладки манипуляции с памятью могут быть небезопасны
- Отладчик уже может видеть DLL, стелс бесполезен
- Упрощает разработку и дебаггинг

#### 2.2. Стирание PE-заголовка (`ErasePEHeader`)

Затирает DOS и NT заголовки DLL в памяти нулями.

**Почему это работает?**
- После загрузки Windows использует только загруженные секции кода/данных
- PE-заголовок не нужен для выполнения уже загруженного модуля
- Анти-чит сканеры часто проверяют память на наличие PE-сигнатур (`MZ`, `PE\0\0`)
- Стирание заголовка делает DLL невидимой для примитивных сканеров памяти

**Меры предосторожности:**
- Проверка валидности указателей через `IsBadReadPtr`
- Валидация магических чисел (`IMAGE_DOS_SIGNATURE`, `IMAGE_NT_SIGNATURE`)
- Проверка разумности `e_lfanew` (смещение не более 4KB)
- Изменение прав доступа памяти через `VirtualProtect`
- SEH exception handling (`__try/__except`) для перехвата нарушений доступа

#### 2.3. Удаление из PEB (`RemoveFromPEB`)

Отвязывает DLL от трех связных списков в PEB (Process Environment Block):
- `InLoadOrderModuleList`
- `InMemoryOrderModuleList`
- `InInitializationOrderModuleList`

**Зачем?**
- Многие анти-читы сканируют PEB для обнаружения подозрительных модулей
- Windows API типа `EnumProcessModules`, `GetModuleHandle` используют эти списки
- После удаления DLL становится невидимой для стандартных средств перечисления модулей

**Почему НЕ удаляем для BlueStacks?**
```cpp
if (!IsBlueStacksProcess())
    RemoveFromPEB(hModule);
```

BlueStacks 5 имеет внутренние механизмы валидации целостности списков модулей. Попытка манипуляции с PEB в процессе BlueStacks может вызвать:
- Нарушение внутренних проверок целостности
- Крэш при перечислении модулей самим приложением
- Конфликт с системами защиты эмулятора

Для BlueStacks достаточно стирания PE-заголовка для базовой обфускации.

### 3. Hook-модуль

#### 3.1. Инициализация MinHook

```cpp
MH_Initialize();
```

MinHook создает «трамплин» для перехвата функций:
1. Сохраняет оригинальные байты функции
2. Записывает в начало функции JMP на наш хук
3. Создает «трамплин» с оригинальными байтами + JMP на продолжение

#### 3.2. Перехват wglSwapBuffers

**Почему именно wglSwapBuffers?**

`wglSwapBuffers` - это функция, которая вызывается **каждый кадр** в OpenGL приложениях для обмена буферов (double buffering). Это идеальное место для:
- Гарантированного вызова каждый кадр
- Доступа к Device Context (HDC), из которого получаем HWND
- Рендеринга нашего overlay до или после основного кадра

```cpp
p_swap_buffers = GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglSwapBuffers");
MH_CreateHook(p_swap_buffers, &wglSwapBuffers, &origin_wglSwapBuffers);
```

**Загрузка opengl32.dll:**
```cpp
HMODULE gl_module = GetModuleHandleA("opengl32.dll");
if (!gl_module)
    gl_module = LoadLibraryA("opengl32.dll");
```

- Сначала проверяем, загружен ли OpenGL модуль
- Если нет - загружаем принудительно (для совместимости с процессами, которые могут задерживать загрузку)
- Это критично для стабильности инжекта в BlueStacks

#### 3.3. Обработка каждого кадра

```cpp
bool __stdcall wglSwapBuffers(HDC hDc)
{
    HWND current_wnd = WindowFromDC(hDc);
    
    // Первый вызов - поиск окна BlueStacks
    if (!wnd_handle && current_wnd && IsWindow(current_wnd))
    {
        char current_class[128];
        GetClassNameA(current_wnd, current_class, sizeof(current_class));
        
        if (strcmp(current_class, "BlueStacksApp") == 0)
        {
            wnd_handle = current_wnd;
            GUI::init(wnd_handle);
        }
    }
    
    // Рендерим GUI, если окно найдено
    if (current_wnd == wnd_handle && GUI::getIsInit())
        GUI::draw();
    
    return origin_wglSwapBuffers(hDc);
}
```

**Почему поиск по имени класса?**
- BlueStacks использует класс окна `"BlueStacksApp"` для своего главного рендер-окна
- Это гарантирует, что мы цепляемся именно к игровому окну, а не к служебным
- Поиск выполняется только один раз при первом кадре

### 4. GUI-модуль

#### 4.1. Инициализация ImGui

```cpp
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO();
io.IniFilename = nullptr;  // Отключаем сохранение настроек
```

**Почему отключаем сохранение?**
- Не создаем файлов на диске (стелс)
- Избегаем ошибок доступа к файловой системе
- Каждый запуск с одинаковыми настройками

#### 4.2. Инициализация backend'ов

```cpp
ImGui_ImplWin32_Init(wnd_handle);
ImGui_ImplOpenGL3_Init(nullptr);
```

- **Win32 Backend**: обрабатывает ввод от Windows (мышь, клавиатура)
- **OpenGL3 Backend**: рендерит ImGui draw lists через OpenGL

**Почему nullptr для OpenGL3?**
- Современные контексты OpenGL не требуют явного указания версии GLSL
- ImGui сам определит доступную версию

#### 4.3. Ручная обработка ввода

```cpp
io.MousePos = ...;
io.MouseDown[i] = (GetAsyncKeyState(vk_buttons[i]) & 0x8000) != 0;
io.DeltaTime = ...;
io.DisplaySize = ...;
```

**Почему не используем стандартный Win32 backend полностью?**
- У нас нет контроля над оконной процедурой (WndProc) BlueStacks
- Не можем перехватывать WM_* сообщения напрямую
- Используем `GetAsyncKeyState` и `GetCursorPos` для опроса состояния

#### 4.4. Цикл рендеринга

```cpp
ImGui_ImplOpenGL3_NewFrame();
// ... настройка io ...
ImGui::NewFrame();
if (do_draw) renderMainInterface();
ImGui::EndFrame();
ImGui::Render();
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
```

Последовательность **критична**:
1. `NewFrame()` - начало нового кадра ImGui
2. Вызовы ImGui API (`ImGui::Begin`, `ImGui::Button`, и т.д.)
3. `EndFrame()` - финализация frame
4. `Render()` - генерация draw lists
5. `RenderDrawData()` - отрисовка через OpenGL

---

## Компоненты системы

### Обфускация

#### LazyImporter
```cpp
LI_FN(GetModuleHandleA)(...)
```
- Разрешает импорты динамически в runtime
- API функции не появляются в Import Table
- Затрудняет статический анализ

#### xorstr
```cpp
const char* target = xorstr_("BlueStacksApp");
```
- XOR-шифрует строки на этапе компиляции
- Дешифруется в runtime при первом использовании
- Строки не видны в статическом анализе бинарника

### Обфускация указателей на функции

```cpp
typedef MH_STATUS(WINAPI* PFN_MH_Initialize)(VOID);
static PFN_MH_Initialize pfn_mh_init = MH_Initialize;
```

**Зачем?**
- Усложняет статический анализ
- Затрудняет обнаружение использования MinHook
- Дополнительный уровень индирекции

---

## Процесс инжекта и инициализации

### Полный timeline

1. **Инжектор** загружает DLL в целевой процесс
2. **DllMain** вызывается с `DLL_PROCESS_ATTACH`
3. Создается рабочий поток (`CreateThread`)
4. **Задержка 100ms** - даем процессу стабилизироваться
5. **Stealth::InitStealth**:
   - Проверка отладчика
   - Стирание PE-заголовка
   - Удаление из PEB (если не BlueStacks)
6. **Задержка 200ms** - даем время на стабилизацию после стелс-операций
7. **Hook::init**:
   - Инициализация MinHook
   - Поиск/загрузка opengl32.dll
   - Создание хука на wglSwapBuffers
8. **Основной цикл**: ожидание нажатия VK_END
9. При каждом SwapBuffers:
   - Поиск окна BlueStacksApp (один раз)
   - Инициализация GUI (один раз при находке окна)
   - Рендеринг UI (каждый кадр)
10. **Shutdown**:
    - Отключение GUI
    - Удаление хуков
    - Выгрузка DLL

### Критические задержки

```cpp
std::this_thread::sleep_for(std::chrono::milliseconds(100)); // До stealth
std::this_thread::sleep_for(std::chrono::milliseconds(200)); // До hook
std::this_thread::sleep_for(std::chrono::milliseconds(50));  // До unload
```

**Зачем нужны задержки?**

1. **100ms до stealth**:
   - Процесс должен завершить инициализацию после загрузки новой DLL
   - Windows loader может еще обрабатывать зависимости
   - Критично для стабильности в эмуляторах типа BlueStacks

2. **200ms до hook**:
   - После манипуляций с памятью (стирание PE, PEB) системе нужно время
   - OpenGL может еще не быть полностью инициализирован
   - BlueStacks может выполнять отложенную инициализацию

3. **50ms перед unload**:
   - Дать GUI время корректно завершить рендеринг
   - Убедиться, что все callback'и завершились
   - Избежать race conditions при выгрузке

### Exception Handling

Все критические секции обернуты в `__try/__except`:

```cpp
__try
{
    // Потенциально опасный код
}
__except (EXCEPTION_EXECUTE_HANDLER)
{
    // Graceful recovery
}
```

**Почему SEH, а не C++ exceptions?**
- SEH перехватывает низкоуровневые исключения Windows (Access Violation, и т.д.)
- C++ exceptions не ловят нарушения доступа к памяти
- Критично для операций с PEB и манипуляций памятью

---

## Технические решения и их обоснование

### Почему стелс-операции могут вызывать крэши?

1. **Конкурентный доступ**: Если BlueStacks перечисляет модули во время удаления из PEB → крэш
2. **Integrity checks**: Современные приложения могут проверять целостность PEB
3. **Timing**: Слишком ранние манипуляции могут поймать процесс в нестабильном состоянии

### Решения для стабильности BlueStacks 5

1. **Отключение RemoveFromPEB** для процессов BlueStacks
2. **Добавление задержек** перед критическими операциями
3. **Валидация** всех указателей перед использованием
4. **Exception handling** вокруг каждой небезопасной операции
5. **Graceful degradation**: если стелс провалился, DLL продолжает работать

### Почему не используется D3D11/D3D12?

BlueStacks использует OpenGL для Android-рендеринга. D3D хуки не сработали бы, так как:
- Android использует OpenGL ES
- BlueStacks транслирует это в desktop OpenGL
- Direct3D не задействован в рендер-пайплайне

---

## Управление

- **INSERT** - показать/скрыть меню
- **END** - выгрузить DLL

---

## Предупреждения

1. Данный код предназначен **только для образовательных целей**
2. Инжект в сторонние процессы может нарушать условия использования ПО
3. Манипуляции с PEB и PE могут быть нестабильны на разных версиях Windows
4. Антивирусы могут детектировать стелс-техники как вредоносное ПО

---

## Системные требования

- Windows 7 и выше (тестировалось на Windows 10/11)
- BlueStacks 5 (или другое OpenGL приложение с классом окна "BlueStacksApp")
- x64 архитектура

---

## Сборка

Используйте CMake:
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Или используйте `build.bat` для быстрой сборки.

---

**Автор**: Educational Project  
**Цель**: Демонстрация техник низкоуровневого программирования под Windows
