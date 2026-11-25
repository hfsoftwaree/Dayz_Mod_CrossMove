// ==================================================
// DADOS DA MORTE (CACHE)
// ==================================================
class GraveCrossDeathData
{
	string playerName;
	string steamId;
	
	string killerName;
	string killerSteamId;
	string weaponName;
}

// ==================================================
// RESTRICTIONS
// ==================================================
class GraveCrossRestriction
{
	string Area_Description;
	string Block_Position; // "x y z"
	float Perimeter;
	ref array<string> Jump_Points;
}

// ==================================================
// CONFIG
// ==================================================
class GraveCrossConfig
{
	int enable;
	string webhook;
	string mapname;
	ref array<ref GraveCrossRestriction> m_RestrictionZones;
}

// ==================================================
// MANAGER
// ==================================================
class GraveCrossManager
	{
		static string FILE_PATH_CONFIG = "$profile:\\HF\\GraveCross\\Config.json";
		static ref GraveCrossConfig Config;
		static ref array<ref GraveCrossDeathData> DeathQueue = new array<ref GraveCrossDeathData>();

		// --------------------------------------------------
		// CARREGA OU ATUALIZA CONFIG
		// --------------------------------------------------
		static void LoadConfig()
		{
			if (!GetGame().IsServer())
				return;

			if (Config)  // Já carregado, não faz nada
				return;

			// Cria o objeto, mas sem inicializar nada manualmente
			Config = new GraveCrossConfig;

			// Apenas ler do arquivo existente
			if (FileExist(FILE_PATH_CONFIG))
			{
				JsonFileLoader<GraveCrossConfig>.JsonLoadFile(FILE_PATH_CONFIG, Config);
			}
			else
			{
				Print("[GraveCross] Config.json não encontrado. O MissionServer deve criar o arquivo.");
				return;
		}
	}

	// --------------------------------------------------
	// RETORNA A RESTRIÇÃO DA ÁREA
	// --------------------------------------------------
	static GraveCrossRestriction GetRestrictionZone(vector pos)
	{
		if (!Config || !Config.enable || !Config.m_RestrictionZones)
			return null;

		foreach (GraveCrossRestriction rz : Config.m_RestrictionZones)
		{
			if (!rz || !rz.Block_Position)
				continue;

			vector blockPos = VectorFromString(rz.Block_Position);
			vector diff = pos - blockPos;
			diff[1] = 0;

			if (diff.Length() <= rz.Perimeter)
				return rz;
		}

		return null;
	}

	// --------------------------------------------------
	// CONVERTE STRING PARA VECTOR
	// --------------------------------------------------
	static vector VectorFromString(string str)
	{
		array<string> parts = new array<string>;
		str.Split(" ", parts);

		if (parts.Count() != 3)
			return Vector(0, 0, 0);

		return Vector(parts[0].ToFloat(), parts[1].ToFloat(), parts[2].ToFloat());
	}

	// --------------------------------------------------
	// ENVIA KILL FEED PARA O WEBHOOK
	// --------------------------------------------------
	//static void SendKillFeedLog(string playerName, string steamId, vector pos)
	static void SendKillFeedLog(GraveCrossDeathData data, vector pos)
	{
		if (!Config || !Config.webhook || Config.webhook == "")
			return;

		int y, mo, d, h, m, s;
		GetYearMonthDay(y, mo, d);
		GetHourMinuteSecond(h, m, s);

		string date = d.ToStringLen(2) + "/" + mo.ToStringLen(2) + "/" + y.ToStringLen(4);
		string time = h.ToStringLen(2) + ":" + m.ToStringLen(2) + ":" + s.ToStringLen(2);

		string posStr = "```X: " + pos[0].ToString() + "    Y: " + pos[1].ToString() + "    Z: " + pos[2].ToString() + "```";
		string izurviveUrl = GetIzurviveLink(pos);

		string payload = "";
		payload += "{\"embeds\":[{";
		payload += "\"title\":\"KILL FEED\",";
		payload += "\"color\":16711680,";
		payload += "\"fields\":[";
		
		// Linha 1: Player | Killed | Weapon
		payload += "{\"name\":\"Player\",\"value\":\"" + data.playerName + "\",\"inline\":true},";
		payload += "{\"name\":\"Steam\",\"value\":\"[" + data.steamId + "](https://steamcommunity.com/profiles/" + data.steamId + ")\",\"inline\":true},";
		payload += "{\"name\":\" \",\"value\":\" \",\"inline\":true},";


		// Linha 2: Steam IDs
		payload += "{\"name\":\"Killed By\",\"value\":\"" + data.killerName + "\",\"inline\":true},";
		if (data.killerSteamId != "")
			payload += "{\"name\":\"Steam Killed\",\"value\":\"[" + data.killerSteamId + "](https://steamcommunity.com/profiles/" + data.killerSteamId + ")\",\"inline\":true},";
		else
			payload += "{\"name\":\"Steam Killed\",\"value\":\"N/A\",\"inline\":true},";
		payload += "{\"name\":\"Weapon\",\"value\":\"" + data.weaponName + "\",\"inline\":true},";

		// Localização e mapa continuam como antes
		payload += "{\"name\":\"Location Cross\",\"value\":\"" + posStr + "\",\"inline\":false},";
		payload += "{\"name\":\"Mapa\",\"value\":\"[Abrir no iZurvive](" + izurviveUrl + ")\",\"inline\":false}";
		payload += "],";
		payload += "\"footer\":{\"text\":\"" + date + " - " + time + "\"}";
		payload += "}]}";

		RestContext ctx = GetRestApi().GetRestContext(Config.webhook);
		if (!ctx) return;

		ctx.SetHeader("application/json");
		ctx.POST(new RestCallback, "", payload);
	}


	// --------------------------------------------------
	// Converte posição do mundo DayZ para iZurvive (Chernarus+)
	// --------------------------------------------------
	static string GetIzurviveLink(vector pos)
	{
		// iZurvive usa X/Z diretamente
		string x = Math.Round(pos[0]).ToString();
		string y = Math.Round(pos[2]).ToString();

		return "https://www.izurvive.com/" + Config.mapname + "/#location=" + x + ";" + y;
	}
}

// ==================================================
// CAPTURA DA MORTE (PLAYER REAL)
// ==================================================
modded class PlayerBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!GetGame().IsServer() || !GetIdentity())
			return;

		GraveCrossDeathData data = new GraveCrossDeathData;
		data.playerName = GetIdentity().GetName();
		data.steamId    = GetIdentity().GetPlainId();

		data.killerName = "Unknown";
		data.killerSteamId = "";
		data.weaponName = "Unknown";

		// Declara a variável de arma uma vez
		EntityAI weapon = null;

		// >>> CAPTURA KILLER
		if (killer)
		{
			// --- SUICIDE ---
			if (killer == this)
			{
				data.killerName = "Suicide";
				data.killerSteamId = "";

				weapon = GetHumanInventory().GetEntityInHands();
				if (weapon)
					data.weaponName = weapon.GetType();
				else
					data.weaponName = "Self"; // fallback se mão estiver vazia
			}
			else
			{
				// --- PLAYER HUMANO ---
				PlayerBase killerPlayer = PlayerBase.Cast(killer);

				// OBJETO ou BOT
				if (!killerPlayer)
				{
					EntityAI killerEntity = EntityAI.Cast(killer);
					if (killerEntity)
						killerPlayer = PlayerBase.Cast(killerEntity.GetHierarchyRootPlayer());
				}

				if (killerPlayer && killerPlayer.GetIdentity())
				{
					data.killerName = killerPlayer.GetIdentity().GetName();
					data.killerSteamId = killerPlayer.GetIdentity().GetPlainId();

					weapon = killerPlayer.GetHumanInventory().GetEntityInHands();
					if (weapon)
						data.weaponName = weapon.GetType();
				}
				else
				{
					// BOT ou NPC
					data.killerName = "BOT";
					data.killerSteamId = "";
					data.weaponName = killer.GetType();
				}
			}
		}
		else
		{
			// --- DANO AMBIENTAL ---
			data.killerName = "Environment";
			data.weaponName = "N/A";
		}
		// <<< FIM CAPTURA KILLER

		GraveCrossManager.DeathQueue.Insert(data);
	}
}


// ==================================================
// CRUZ (POSIÇÃO FINAL)
// ==================================================
modded class Expansion_Gravecross
{
	override void EEInit()
	{
		super.EEInit();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MoveBasedOnConfig, 1200, false);
	}

	void MoveBasedOnConfig()
	{
		if (!GetGame().IsServer())
			return;

		vector originalPos = GetPosition();

		GraveCrossManager.LoadConfig();
		GraveCrossRestriction rz = GraveCrossManager.GetRestrictionZone(originalPos);

		vector targetPos = originalPos;

		if (rz && rz.Jump_Points && rz.Jump_Points.Count() > 0)
		{
			// Escolhe um jump point aleatório da lista
			int idx = Math.RandomIntInclusive(0, rz.Jump_Points.Count() - 1);
			targetPos = GraveCrossManager.VectorFromString(rz.Jump_Points[idx]);

			// Aplica pequeno deslocamento aleatório para evitar sobreposição
			float offsetX = Math.RandomFloatInclusive(-2.0, 2.0);
			float offsetZ = Math.RandomFloatInclusive(-2.0, 2.0);
			targetPos = Vector(
				targetPos[0] + offsetX,
				targetPos[1],
				targetPos[2] + offsetZ);

			SetPosition(targetPos);
			SetOrientation(Vector(0, 0, 0));
		}

		// Declara a variável para pegar dados do player
		GraveCrossDeathData data = null;

		if (GraveCrossManager.DeathQueue.Count() > 0)
		{
			data = GraveCrossManager.DeathQueue.Get(0);
			GraveCrossManager.DeathQueue.Remove(0);

			GraveCrossManager.SendKillFeedLog(data, targetPos);

			ExpansionMarkerModule module = ExpansionMarkerModule.GetModuleInstance();

			if (module && module.IsMissionHost())
			{
				PlayerBase player = Expansion_Gravecross.GetPlayerBySteamID(data.steamId);

				if (player && player.GetIdentity())
				{
					module.CreateDeathMarkerServer(targetPos, player.GetIdentity());
					Print("[GraveCross] Personal Death Marker enviado!");
				}
			}
		}

			// Salva loot apenas se data existir
			if (data)
			{
				GraveCrossLootLogger.SaveLoot(data.steamId, data.playerName, this, 86400);
				Print("[GraveCross] Loot salvo para: " + data.playerName);
			}
			else
			{
				Print("[GraveCross] Nenhum player na fila, loot não salvo.");
			}
		}
		//End
		

	
	// --------------------------------------------------
	// Busca PlayerBase pelo SteamID
	// --------------------------------------------------
	static PlayerBase GetPlayerBySteamID(string steamId)
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man p : players)
		{
			PlayerBase player = PlayerBase.Cast(p);
			if (!player) continue;
			if (!player.GetIdentity()) continue;
			if (player.GetIdentity().GetPlainId() == steamId) return player;
		}
		return null;
	}
}

// ==================================================
// CRUZ (POSIÇÃO FINAL) – LowLifetime
// ==================================================
modded class Expansion_Gravecross_LowLifetime
{
	override void EEInit()
	{
		super.EEInit();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MoveBasedOnConfig, 1200, false);
	}

	void MoveBasedOnConfig()
	{
		if (!GetGame().IsServer())
			return;

		vector originalPos = GetPosition();

		GraveCrossManager.LoadConfig();
		GraveCrossRestriction rz = GraveCrossManager.GetRestrictionZone(originalPos);

		vector targetPos = originalPos;

		if (rz && rz.Jump_Points && rz.Jump_Points.Count() > 0)
		{
			int idx = Math.RandomIntInclusive(0, rz.Jump_Points.Count() - 1);
			targetPos = GraveCrossManager.VectorFromString(rz.Jump_Points[idx]);

			float offsetX = Math.RandomFloatInclusive(-2.0, 2.0);
			float offsetZ = Math.RandomFloatInclusive(-2.0, 2.0);

			targetPos = Vector(
				targetPos[0] + offsetX,
				targetPos[1],
				targetPos[2] + offsetZ);

			SetPosition(targetPos);
			SetOrientation(Vector(0, 0, 0));
		}

		GraveCrossDeathData data = null;

		if (GraveCrossManager.DeathQueue.Count() > 0)
		{
			data = GraveCrossManager.DeathQueue.Get(0);
			GraveCrossManager.DeathQueue.Remove(0);

			GraveCrossManager.SendKillFeedLog(data, targetPos);

			// ===== PERSONAL MARKER =====
			ExpansionMarkerModule module = ExpansionMarkerModule.GetModuleInstance();

			if (module && module.IsMissionHost())
			{
				PlayerBase player = Expansion_Gravecross.GetPlayerBySteamID(data.steamId);

				if (player && player.GetIdentity())
				{
					module.CreateDeathMarkerServer(targetPos, player.GetIdentity());
					Print("[GraveCross] Personal Death Marker enviado (LowLifetime)!");
				}
			}
		}

		if (data)
		{
			GraveCrossLootLogger.SaveLoot(data.steamId, data.playerName, this, 86400);
			Print("[GraveCross] Loot salvo para: " + data.playerName);
		}
		else
		{
			Print("[GraveCross] Nenhum player na fila, loot não salvo.");
		}
	}
}




