#include "Material.h"


Material::Material(const sf::String& name, MaterialType materialType)
	: Item(name, ItemType::KeyItem), m_materialType(materialType)
{

}
