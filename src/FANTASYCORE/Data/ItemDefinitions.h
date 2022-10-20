#pragma once

enum FC_ITEM_LIST
{
	ITEM_NONE = -1,
	ITEM_KNIGHT_CREST,
	ITEM_KNIGHT_FLAG,
	ITEM_KNIGHT_HEADPIECE,
	ITEM_KNIGHT_MIND, //name changed
	ITEM_FURY_NECKLACE,
	ITEM_FURY_CHARM,
	ITEM_FURY_SHORT_SWORD,
	ITEM_RAGE,
	ITEM_BLESS_HEADBAND,
	ITEM_BLESS_BANGLE,
	ITEM_BLESS_RING,
	ITEM_ANGEL_BLESSING, //name changed
	ITEM_CURSE_NECKLACE,
	ITEM_CURSE_SHOE,
	ITEM_CURSE_SHIELD,
	ITEM_GODDESS_FLAG,
	ITEM_UNLUCKY_HELMET,
	ITEM_UNLUCKY_GLOVE,
	ITEM_UNLUCKY_RING,
	ITEM_GODDESS_CROWN,
	ITEM_BROKEN_STATUE_ARM,
	ITEM_BROKEN_STATUE_LEG,
	ITEM_BROKEN_STATUE_BODY,
	ITEM_BROKEN_STATUE_HEAD,
	ITEM_GODDESS_STATUE,
	ITEM_GODDESS_DIVINE,
	ITEM_DYING_GLASSES,
	ITEM_DYING_GLOVE,
	ITEM_KNUCKLE,
	ITEM_ASSASIN_GLOVE_1,
	ITEM_ASSASIN_GLOVE_2,
	ITEM_DEADLY_RING,
	ITEM_CAVALRY_SECRET_BOOK,
	ITEM_DEADLY_BANGLE,
	ITEM_LIFE_CRYSTAL,
	ITEM_FURY_RING,
	ITEM_GIANT_CLUB,
	ITEM_RED_HORSE_PANDENT,
	ITEM_SAND_GLASS,
	ITEM_RED_PORTION_L,
	ITEM_RED_PORTION_M,
	ITEM_RED_PORTION_S,
	ITEM_WHITE_WOOD,
	ITEM_BLUE_PORTION,
	ITEM_BLUE_PORTION_S,
	ITEM_EMBLEM,
	ITEM_ORB_MARK,
	ITEM_GRAIL,
	ITEM_WEAPON_MARK,
	ITEM_ARMOR_MARK,
	ITEM_FLYING_SHOES_MARK,
	ITEM_ANGLE_WING_MARK,
	ITEM_BAG,
	ITEM_WEAPON_BAG,
	ITEM_HORSE_STATUE,
	ITEM_EXP_2_MARK,
	ITEM_BONUS_EXP_2_MARK,
	ITEM_WHISTLE,
	ITEM_BENUS_STATUE,
	ITEM_WHITE_PORTION_S,
	ITEM_WHITE_PORTION_M,
	ITEM_WHITE_PORTION_L,
	ITEM_CHURCH_KNIGHT_SWORD,
	ITEM_BLOODY_ROSE,
	ITEM_FIRE_LONG_SWORD,
	ITEM_ORB_HOLY_SWORD,
	ITEM_PHENIX_BLADE,
	ITEM_AMESIST_SAVER,
	ITEM_PROMINUNS,
	ITEM_CROW_SORAS,
	ITEM_CHURCH_KNIGHT_SPEAR,
	ITEM_EARTH_PIKE,
	ITEM_KNIGHT_LANS,
	ITEM_WATER_SPIKE,
	ITEM_BLUE_RANSOR,
	ITEM_CRYSTAL_SPEAR,
	ITEM_SEA_GOD_SPEAR,
	ITEM_BORYUNARK,
	ITEM_MAGIC_SCHOOL_WAND,
	ITEM_MAGICIAN_WAND,
	ITEM_ANTIC_WAND,
	ITEM_MANA_WAND,
	ITEM_PRIME_KEY,
	ITEM_MAGICIAN_LORD_WAND,
	ITEM_RAINBOW_WAND,
	ITEM_DOUBLE_EDGE,
	ITEM_DOUBLE_SWORD,
	ITEM_TORNADO,
	ITEM_CRECENT_DUAL,
	ITEM_BLACK_OKS,
	ITEM_ENSOROR,
	ITEM_TEMPEST,
	ITEM_PANG_DAGGER,
	ITEM_ASSASIN_SHORT_SWORD,
	ITEM_KOO_CRI,
	ITEM_BLACK_SCIMITAR,
	ITEM_NEMESIS,
	ITEM_HELL_SHORT_SWORD,
	ITEM_DEVIL_TOOS,
	ITEM_OLD_ECON,
	ITEM_ROJARIO,
	ITEM_SECRAMENT,
	ITEM_SENTINEL,
	ITEM_CROOKS,
	ITEM_TRUST_WAND,
	ITEM_GLORIA,
	ITEM_GUARD_CHARM_LV1,
	ITEM_GUARD_CHARM_LV2,
	ITEM_GUARD_CHARM_LV3,
	ITEM_GUARD_CHARM_LV4,
	ITEM_GUARD_CHARM_LV5,
	ITEM_GUARD_CHARM_LV6,
	ITEM_GUARD_CHARM_LV7,
	ITEM_GUARD_CHARM_LV8,
	ITEM_GIANT_BANGLE_LV1,
	ITEM_GIANT_BANGLE_LV2,
	ITEM_GIANT_BANGLE_LV3,
	ITEM_GIANT_BANGLE_LV4,
	ITEM_GIANT_BANGLE_LV5,
	ITEM_GIANT_BANGLE_LV6,
	ITEM_GIANT_BANGLE_LV7,
	ITEM_GIANT_BANGLE_LV8,
	ITEM_HEALTH_BEAD_LV1,
	ITEM_HEALTH_BEAD_LV2,
	ITEM_HEALTH_BEAD_LV3,
	ITEM_HEALTH_BEAD_LV4,
	ITEM_HEALTH_BEAD_LV5,
	ITEM_HEALTH_BEAD_LV6,
	ITEM_HEALTH_BEAD_LV7,
	ITEM_HEALTH_BEAD_LV8,
	ITEM_FOUR_LEAF_CLOVER_LV1,
	ITEM_FOUR_LEAF_CLOVER_LV2,
	ITEM_FOUR_LEAF_CLOVER_LV3,
	ITEM_GRIFFON_FEATHER_LV1,
	ITEM_GRIFFON_FEATHER_LV2,
	ITEM_DANCER_GLOVE_LV1,
	ITEM_DANCER_GLOVE_LV2,
	ITEM_ORB_CHARM_LV1,
	ITEM_ORB_CHARM_LV2,
	ITEM_ORB_CHARM_LV3,
	ITEM_WISDOM_CHARM_LV1,
	ITEM_WISDOM_CHARM_LV2,
	ITEM_WISDOM_CHARM_LV3,
	ITEM_VALOR_FLAG_LV1,
	ITEM_VALOR_FLAG_LV2,
	ITEM_VALOR_FLAG_LV3,
	ITEM_VALOR_FLAG_LV4,
	ITEM_VALOR_FLAG_LV5,
	ITEM_VALOR_FLAG_LV6,
	ITEM_VALOR_FLAG_LV7,
	ITEM_VALOR_FLAG_LV8,
	ITEM_DEFENCE_FLAG_LV1,
	ITEM_DEFENCE_FLAG_LV2,
	ITEM_DEFENCE_FLAG_LV3,
	ITEM_WIND_MANTLE_LV1,
	ITEM_WIND_MANTLE_LV2,
	ITEM_WIND_MANTLE_LV3,
	ITEM_WIND_MANTLE_LV4,
	ITEM_WIND_MANTLE_LV5,
	ITEM_SADDLE_LV1,
	ITEM_SADDLE_LV2,
	ITEM_SADDLE_LV3,
	ITEM_SADDLE_LV4,
	ITEM_SADDLE_LV5,
	ITEM_SADDLE_LV6,
	ITEM_SADDLE_LV7,
	ITEM_SADDLE_LV8,
	ITEM_THEIF_BOOTS_LV1,
	ITEM_THEIF_BOOTS_LV2,
	ITEM_DANCER_GLOVE_LV3,
	ITEM_TYPE_LAST
};


enum ITEM_TYPE
{
	//NORMAL , WEAPON, ACCESSORY
	ITP_NORMAL,
	ITP_ACCE,
	ITP_ACCE_SET,
	ITP_ACCE_SET_RLT,
	ITP_ACCE_UNIQUE,
	ITP_ACCE_COL,
	ITP_ACCE_COL_RLT,
	ITP_WEAPON,
	ITP_LAST
};
