class GraveCrossLootItem
{
	string Type;
	float Health;
	float Quantity;
	int IsAttachment;

	ref array<ref GraveCrossLootItem> Children;
}

class GraveCrossLootFile
{
	string OwnerSteamID;
	string OwnerName;
	vector Position;
	vector Orientation;
	int ExpireEpoch;

	ref array<ref GraveCrossLootItem> RootItems;
}
