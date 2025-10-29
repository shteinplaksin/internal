#ifndef CONFIG_H_
#define CONFIG_H_

struct VisualSettings
{
	bool box{ true };
	bool skeleton{ true };
	bool glow{ false };
	bool healthBar{ true };
	float boxColor[4]{ 1.0f, 0.5f, 0.0f, 1.0f };
	float skeletonColor[4]{ 1.0f, 0.35f, 0.0f, 0.9f };
	float glowColor[4]{ 1.0f, 0.65f, 0.0f, 0.8f };
	float healthColor[4]{ 0.2f, 1.0f, 0.2f, 1.0f };
	float visualFov{ 90.0f };
};

struct RageSettings
{
	bool enableRageBot{ true };
	bool enableAntiAim{ true };
	bool autoshoot{ false };
	float hitchance{ 65.0f };
	float resolverStrength{ 50.0f };
};

struct LegitSettings
{
	bool enableTrigger{ true };
	bool boneHead{ true };
	bool boneNeck{ false };
	bool boneChest{ true };
	bool boneStomach{ false };
	bool useSmooth{ true };
	float smoothValue{ 2.5f };
	float fov{ 3.0f };
};

struct MiscSettings
{
	bool bunnyHop{ true };
	bool noFlash{ true };
	bool radarHack{ false };
	float nightMode{ 0.5f };
};

struct SettingsData
{
	int selectedConfig{ 0 };
	bool autoSave{ true };
};

struct InfoData
{
	bool confirmExit{ false };
};

extern RageSettings g_rageSettings;
extern VisualSettings g_visualSettings;
extern LegitSettings g_legitSettings;
extern MiscSettings g_miscSettings;
extern SettingsData g_settingsData;
extern InfoData g_infoData;
extern int g_activeTab;
extern bool g_anyEspEnabled;

#endif

