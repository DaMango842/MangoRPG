#include "KeyItem.h"


KeyItem::KeyItem(const sf::String& name, KeyItemType keyItemType)
	: Item(name, ItemType::KeyItem), m_keyItemType(keyItemType)
{
}
