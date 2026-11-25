class GraveCrossLootLogger
{
	static string LOOT_PATH = "$profile:\\HF\\GraveCross\\Loot\\";

	// --------------------------------------------------
	// SALVA O LOOT COMPLETO DA CRUZ
	// --------------------------------------------------
	static void SaveLoot(
		string ownerSteamId,
		string ownerName,
		EntityAI graveCross,
		int lifetimeSeconds
	)
	{
		if (!graveCross || ownerSteamId == "")
			return;

		int now = Math.Floor(GetGame().GetTime() / 1000);
		int expireEpoch = now + lifetimeSeconds;

		GraveCrossLootFile data = new GraveCrossLootFile;
		data.OwnerSteamID = ownerSteamId;
		data.OwnerName    = ownerName;
		data.Position     = graveCross.GetPosition();
		data.Orientation  = graveCross.GetOrientation();
		data.ExpireEpoch  = expireEpoch;
		data.RootItems    = new array<ref GraveCrossLootItem>;

		CollectInventory(graveCross, data.RootItems);

		if (!FileExist(LOOT_PATH))
			MakeDirectory(LOOT_PATH);

		string fileName = LOOT_PATH + ownerSteamId + "_" + expireEpoch.ToString() + ".json";

		JsonFileLoader<GraveCrossLootFile>.JsonSaveFile(fileName, data);
	}

	// --------------------------------------------------
	// COLETA INVENTÁRIO (RECURSIVO)
	// --------------------------------------------------
	static void CollectInventory(EntityAI entity, out array<ref GraveCrossLootItem> items)
	{
		if (!entity)
			return;

		// CARGO
		CargoBase cargo = entity.GetInventory().GetCargo();
		if (cargo)
		{
			for (int i = 0; i < cargo.GetItemCount(); i++)
			{
				EntityAI item = cargo.GetItem(i);
				items.Insert(ExtractItem(item));
			}
		}

		// ATTACHMENTS
		for (int a = 0; a < entity.GetInventory().AttachmentCount(); a++)
		{
			EntityAI att = entity.GetInventory().GetAttachmentFromIndex(a);
			items.Insert(ExtractItem(att));
		}
	}

	// --------------------------------------------------
	// EXTRAI ITEM
	// --------------------------------------------------
	static GraveCrossLootItem ExtractItem(EntityAI item)
{
    GraveCrossLootItem li = new GraveCrossLootItem;
    li.Type = item.GetType();
    li.Health = item.GetHealth();

    if (item.HasQuantity())
        li.Quantity = item.GetQuantity();
    else
        li.Quantity = 1;

    InventoryLocation loc = new InventoryLocation;
	item.GetInventory().GetCurrentInventoryLocation(loc);
	if (loc.GetType() == InventoryLocationType.ATTACHMENT) li.IsAttachment = 1; else li.IsAttachment = 0;


    li.Children = new array<ref GraveCrossLootItem>;

    CollectInventory(item, li.Children);
    return li;
}
}
