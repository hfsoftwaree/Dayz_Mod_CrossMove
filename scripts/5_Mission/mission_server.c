modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();

        if (!GetGame().IsServer())
            return;

        string folder = "$profile:\\HF\\GraveCross\\";
        if (!FileExist(folder))
            MakeDirectory(folder);

        string pathConfig = folder + "Config.json";
        bool configModified = false;
        GraveCrossConfig loadedConfig = new GraveCrossConfig;

        if (FileExist(pathConfig))
        {
            JsonFileLoader<GraveCrossConfig>.JsonLoadFile(pathConfig, loadedConfig);
            Print("[GraveCross] Config.json carregado.");
        }
        else
        {
            loadedConfig = new GraveCrossConfig;
            // NÃO definir enable aqui!
            loadedConfig.webhook = "COLE_AQUI_SEU_WEBHOOK";
            loadedConfig.mapname = "chernarusplus";
            loadedConfig.m_RestrictionZones = new array<ref GraveCrossRestriction>;
			GraveCrossRestriction defaultRz = new GraveCrossRestriction;
			defaultRz.Area_Description = "";
			defaultRz.Block_Position = "";
			defaultRz.Perimeter = 0;
			defaultRz.Jump_Points = new array<string>;
			loadedConfig.m_RestrictionZones.Insert(defaultRz);
            configModified = true;
            Print("[GraveCross] Config.json inexistente — criando novo.");
        }

        // Preenche apenas campos ausentes, sem tocar no enable
        if (!loadedConfig.webhook || loadedConfig.webhook == "")
        {
            loadedConfig.webhook = "COLE_AQUI_SEU_WEBHOOK";
            configModified = true;
        }

        if (!loadedConfig.mapname || loadedConfig.mapname == "")
        {
            loadedConfig.mapname = "chernarusplus";
            configModified = true;
        }

        if (!loadedConfig.m_RestrictionZones)
        {
            loadedConfig.m_RestrictionZones = new array<ref GraveCrossRestriction>;
            configModified = true;
        }

        if (configModified || !FileExist(pathConfig))
        {
            JsonFileLoader<GraveCrossConfig>.JsonSaveFile(pathConfig, loadedConfig);
            Print("[GraveCross] Config.json atualizado automaticamente.");
        }
    }
}