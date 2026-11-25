class CfgPatches
{
    class Expansion_Gravecross
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Consumables"};
    };
};

class CfgMods
{
    class Expansion_Gravecross
    {
        dir="ZHF_CrossMove";
        picture="";
        action="";
        hideName=0;
        hidePicture=1;
        name="ZHF_CrossMove";
        credits="HF";
        author="HF";
        authorID="0";
        version="1.0";
        extra=0;
        type="mod";
        dependencies[]=
       {
            "World", "mission"
        };
        class defs
        {
            class worldScriptModule
            {
                value="";
                files[]=
                {
                    "ZHF_CrossMove/scripts/4_World"
                };
            };
			
			class missionScriptModule
            {
                value="";
                files[]=
                {
                    "ZHF_CrossMove/scripts/5_Mission"
                };
            };
        };
    };
};
