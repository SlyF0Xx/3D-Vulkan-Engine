#include "IMaterial.h"

static int material_id = 0;

IMaterial::IMaterial()
	: id(material_id++)
{}

int IMaterial::get_start_id()
{
	return material_id;
}