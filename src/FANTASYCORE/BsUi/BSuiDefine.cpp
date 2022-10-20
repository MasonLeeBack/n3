#include "StdAfx.h"
#include "BsUiDefine.h"


PropertyDefine g_Prpt_Misc[] = 
{
	{"Name",			_CPT_S_NAME,			_VT_CHAR,	"Control Name", TRUE},
	{"Type",			_CPT_S_TYPE,			_VT_CHAR,	"Control Type", FALSE},
	{"ID",				_CPT_S_ID,				_VT_CHAR,	"Control ID", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_AttrSetting[] = 
{
	{"Enable Focus",	_CPT_B_ENABLE,			_VT_BOOL,	"Enable focus On/Off", TRUE},
	{"Enable Move",		_CPT_B_MOVABLE,			_VT_BOOL,	"Movable On/Off", TRUE},
	{"Show",			_CPT_B_SHOW,			_VT_BOOL,	"Show On/Off", TRUE},
	{"Drag and Drop",	_CPT_B_DRAG_N_DROP,		_VT_BOOL,	"Drag and Drop On/Off", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_PosSize[] = 
{
	{"Absolute Pos X",	_CPT_I_AX,				_VT_INT,	"Absolute Coordinates X", TRUE},
	{"Absolute Pos Y",	_CPT_I_AY,				_VT_INT,	"Absolute Coordinates Y", TRUE},
	{"Width",			_CPT_I_WIDTH,			_VT_INT,	"Control Width", TRUE},
	{"Height",			_CPT_I_HEIGHT,			_VT_INT,	"Control Height", TRUE},
	{"Relative Pos X",	_CPT_I_RX,				_VT_INT,	"Relative Coordinates X", TRUE},
	{"Relative Pos Y",	_CPT_I_RY,				_VT_INT,	"Relative Coordinates Y", TRUE},

	{NULL},
};


//text Info
PropertyDefine g_Prpt_TextInfoType1[] = 
{
	{"Text Id",			_CPT_BT_TEXTTABLE,			_VT_BUTTON, "", TRUE},
	{"Text Pos X",		_CPT_I_TEXT_POS_X,			_VT_INT,	"Text Position X", TRUE},
	{"Text Pos Y",		_CPT_I_TEXT_POS_Y,			_VT_INT,	"Text Position Y", TRUE},
	{"Selected Font",	_CPT_E_SELECT_FA,			_VT_FONT,	"Selected font type", TRUE},
	{"Defaulted Font",	_CPT_E_DEFAULT_FA,			_VT_FONT,	"Defaulted font type", TRUE},
	{"Disable Font",	_CPT_E_DISABLE_FA,			_VT_FONT,	"Disable font type", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_TextInfoType2[] = 
{	
	{"Text Pos X",		_CPT_I_TEXT_POS_X,			_VT_INT,	"Text Position X", TRUE},
	{"Text Pos Y",		_CPT_I_TEXT_POS_Y,			_VT_INT,	"Text Position Y", TRUE},
	{"Selected Font",	_CPT_E_SELECT_FA,			_VT_FONT,	"Selected font type", TRUE},
	{"Defaulted Font",	_CPT_E_DEFAULT_FA,			_VT_FONT,	"Defaulted font type", TRUE},
	{"Disable Font",	_CPT_E_DISABLE_FA,			_VT_FONT,	"Disable font type", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_TextInfoType3[] = 
{	
	{"Text Id",			_CPT_BT_TEXTTABLE,			_VT_BUTTON, "", TRUE},
	{"Font Type",		_CPT_E_DISABLE_FA,			_VT_FONT,	"Font type", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_Button[] = 
{
	{"Button Mode",		_CPT_E_BUTTON_MODE,		_VT_ENUM,	"Button Mode", TRUE},
	{"Group Id",		_CPT_I_GROUPID,			_VT_INT,	"Group Index", TRUE},
	{"Check",			_CPT_B_CHECK,			_VT_BOOL,	"Check On/Off", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_Listbox[] = 
{
	{"Lines",			_CPT_I_LINE,			_VT_INT,	"List Line Count", TRUE},
	{"Show Sel box",	_CPT_B_SEL_BOX,			_VT_BOOL,	"Show Selected Item Box", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_SelBoxUVMode[] =
{
	{"Select UV",		_CPT_BT_SEL_SB_UV,		_VT_BUTTON,	"Select UV Image ", TRUE},
	{"Default UV",		_CPT_BT_DEF_SB_UV,		_VT_BUTTON,	"Default UV Image ", TRUE},
	{"Disable UV",		_CPT_BT_DIS_SB_UV,		_VT_BUTTON,	"Disable UV Image ", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_SelBoxBlockMode[] =
{
	{"Select Block",	_CPT_E_SEL_SB_BLOCK,	_VT_BLOCKTABLE,	"Select Block Image", TRUE},
	{"Default Block",	_CPT_E_DEF_SB_BLOCK,	_VT_BLOCKTABLE,	"Default Block Image", TRUE},
	{"Disable Block",	_CPT_E_DIS_SB_BLOCK,	_VT_BLOCKTABLE,	"Disable Block Image", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_ListCtrl[] = 
{
	{"Lines",			_CPT_I_LINE,			_VT_INT,	"List Line Count", TRUE},
	{"Column",			_CPT_I_COLUMN,			_VT_INT,	"List Column Count", TRUE},
	{"Show Sel box",	_CPT_B_SEL_BOX,			_VT_BOOL,	"Show Selected Item Box", TRUE},

	{NULL},
};


PropertyDefine g_Prpt_ImageMode[] =
{
	{_tag_IMGMODE,		_CPT_E_IMAGE_MODE,			_VT_ENUM,		_tag_IMGMODE, TRUE},
	{"Actual Size",		_CPT_BT_IMAGE_ACTUAL_SIZE,	_VT_BUTTON,		"Makes at actual size.", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_ImageUVMode[] =
{	
	{"Select UV",		_CPT_BT_SEL_IMAGE_UV,	_VT_BUTTON,		"Select UV Image ", TRUE},
	{"Default UV",		_CPT_BT_DEF_IMAGE_UV,	_VT_BUTTON,		"Default UV Image ", TRUE},
	{"Disable UV",		_CPT_BT_DIS_IMAGE_UV,	_VT_BUTTON,		"Disable UV Image ", TRUE},

	{NULL},
};

/*
PropertyDefine g_Prpt_ImageBlockMode[] =
{
	{"Select Block",	_CPT_E_SEL_IMAGE_BLOCK,	_VT_BLOCKTABLE,	"Select Block Image", TRUE},
	{"Default Block",	_CPT_E_DEF_IMAGE_BLOCK,	_VT_BLOCKTABLE,	"Default Block Image", TRUE},
	{"Disable Block",	_CPT_E_DIS_IMAGE_BLOCK,	_VT_BLOCKTABLE,	"Disable Block Image", TRUE},

	{NULL},
};
*/

PropertyDefine g_Prpt_ImageImgCtrl[] =
{	
	{"UV Image",		_CPT_BT_DIS_IMAGE_UV,	_VT_BUTTON,		"UV Image ", TRUE},
	//{"Block Image",		_CPT_E_DIS_IMAGE_BLOCK,	_VT_BLOCKTABLE,	"Block Image", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_ImageSlideNormal[] =
{
	{"Select",		_CPT_BT_SEL_BAR_UV,			_VT_BUTTON,	"Select Bar UV Image", TRUE},
	{"Default",		_CPT_BT_DEF_BAR_UV,			_VT_BUTTON,	"Default Bar UV Image", TRUE},
	{"Disable",		_CPT_BT_DIS_BAR_UV,			_VT_BUTTON,	"Disable Bar UV Image", TRUE},
	{"Line",		_CPT_BT_SLIDER_LINE_UV,		_VT_BUTTON,	"Slider Line Image", TRUE},
	{"Base",		_CPT_BT_SLIDER_BASE_UV,		_VT_BUTTON,	"Slider Base Image", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_ImageSlideEtc[] =
{
	{"Bar1 - Select",	_CPT_BT_SEL_BAR1_UV,		_VT_BUTTON,	"Select Bar UV Image", TRUE},
	{"------ Default",	_CPT_BT_DEF_BAR1_UV,		_VT_BUTTON,	"Default Bar UV Image", TRUE},
	{"------ Disable",	_CPT_BT_DIS_BAR1_UV,		_VT_BUTTON,	"Disable Bar UV Image", TRUE},
	{"Bar2 - Select",	_CPT_BT_SEL_BAR2_UV,		_VT_BUTTON,	"Select Bar UV Image", TRUE},
	{"------ Default",	_CPT_BT_DEF_BAR2_UV,		_VT_BUTTON,	"Default Bar UV Image", TRUE},
	{"------ Disable",	_CPT_BT_DIS_BAR2_UV,		_VT_BUTTON,	"Disable Bar UV Image", TRUE},
	{"Base",			_CPT_BT_SLIDER_BASE_UV,		_VT_BUTTON,	"Slider Base Image", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_ImageSlideGauge[] =
{
	{"Select",		_CPT_BT_SEL_BAR_UV,		_VT_BUTTON,	"Select Bar UV Image", TRUE},
	{"Default",		_CPT_BT_DEF_BAR_UV,		_VT_BUTTON,	"Default Bar UV Image", TRUE},
	{"Disable",		_CPT_BT_DIS_BAR_UV,		_VT_BUTTON,	"Disable Bar UV Image", TRUE},
	{"Base",		_CPT_BT_SLIDER_BASE_UV,	_VT_BUTTON,	"Slider Base Image", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_ImageFile[] =
{
	{"Filename",		_CPT_BT_IMAGE_FILE,		_VT_BUTTON,		"Texture Filename", TRUE},
	{"Local",			_CPT_B_IMAGE_LOCAL,		_VT_BOOL,		"Localizes Texture", TRUE},
	{"U1",				_CPT_I_IMAGE_U1,		_VT_INT,		"U1 Coordinates", TRUE},
	{"V1",				_CPT_I_IMAGE_V1,		_VT_INT,		"V1 Coordinates", TRUE},
	{"U2",				_CPT_I_IMAGE_U2,		_VT_INT,		"U2 Coordinates", TRUE},
	{"V2",				_CPT_I_IMAGE_V2,		_VT_INT,		"V2 Coordinates", TRUE},
	{"Real Size",		_CPT_B_IMAGE_SIZE,		_VT_BOOL,		"Check On/Off", TRUE},
	{"Reverse",			_CPT_B_IMAGE_REVERSE,	_VT_BOOL,		"Check On/Off", TRUE},
	{"Rotate",			_CPT_I_IMAGE_ROTATE,	_VT_INT,		"Radian 0 - 360", TRUE},

	{NULL},
};





PropertyDefine g_Prpt_Slider[] =
{	
	{"Type",		_CPT_E_SLIDER_TYPE,			_VT_ENUM,		"Slider Type", TRUE},
	{"Orientation",	_CPT_E_SLIDER_ORIENTATION,	_VT_ENUM,		"One of : Horizonal, Vertical", TRUE},
	{"Min Range",	_CPT_I_SLIDER_MIN,			_VT_INT,		"Slider Min Range", TRUE},
	{"Max Range",	_CPT_I_SLIDER_MAX,			_VT_INT,		"Slider Max Range", TRUE},
	{"Bar Width",	_CPT_I_SLIDER_BAR_W,		_VT_INT,		"Slider Bar Width", TRUE},
	{"Bar Height",	_CPT_I_SLIDER_BAR_H,		_VT_INT,		"Slider BAr Height", TRUE},

	{NULL},
};

//-----------------------------------------------------------------------------------------
ParameterDefine g_Param_Layer[] = 
{
	{_tag_MISC,					g_Prpt_Misc,				"Control Misc Infomation"},
	{_tag_ATTR,					g_Prpt_AttrSetting,			"Control Attribute Info"},
	{_tag_POS_SIZE,				g_Prpt_PosSize,				"Control Position and Size Info"},

	{_tag_IMGMODE,				g_Prpt_ImageMode,			"Control Image Mode"},
	{_tag_UVMODE,				g_Prpt_ImageUVMode,			"Control Image UV Mode"},
	//{_tag_BLOCKMODE,			g_Prpt_ImageBlockMode,		"Control Image Block Mode"},
	
	{NULL},
};

ParameterDefine g_Param_Button[] = 
{
	{_tag_MISC,					g_Prpt_Misc,				"Control Misc Infomation"},
	{_tag_ATTR,					g_Prpt_AttrSetting,			"Control Attribute Info"},
	{_tag_POS_SIZE,				g_Prpt_PosSize,				"Control Position and Size Info"},
	{_tag_BUTTON,				g_Prpt_Button,				"Control Button Infomation"},
	{_tag_TEXT,					g_Prpt_TextInfoType1,		"Text Infomation"},

	{_tag_IMGMODE,				g_Prpt_ImageMode,			"Control Image Mode"},
	{_tag_UVMODE,				g_Prpt_ImageUVMode,			"Control Image UV Mode"},
	//{_tag_BLOCKMODE,			g_Prpt_ImageBlockMode,		"Control Image Block Mode"},
	
	{NULL},
};

ParameterDefine g_Param_Listbox[] = 
{
	{_tag_MISC,					g_Prpt_Misc,				"Control Misc Infomation"},
	{_tag_ATTR,					g_Prpt_AttrSetting,			"Control Attribute Info"},
	{_tag_POS_SIZE,				g_Prpt_PosSize,				"Control Position and Size Info"},
	{_tag_LIST,					g_Prpt_Listbox,				"Control List Infomation"},
	{_tag_TEXT,					g_Prpt_TextInfoType2,		"Text Infomation"},
	{_tag_IMGMODE,				g_Prpt_ImageMode,			"Control Image Mode"},
	{_tag_LBS_UVMODE,			g_Prpt_SelBoxUVMode,		"Control List Select Box UV"},
	{_tag_LBS_BKMODE,			g_Prpt_SelBoxBlockMode,		"Control List Select Box Block"},
	{_tag_UVMODE,				g_Prpt_ImageUVMode,			"Control Image UV Mode"},
	//{_tag_BLOCKMODE,			g_Prpt_ImageBlockMode,		"Control Image Block Mode"},
	
	{NULL},
};

ParameterDefine g_Param_ListCtrl[] = 
{
	{_tag_MISC,					g_Prpt_Misc,				"Control Misc Infomation"},
	{_tag_ATTR,					g_Prpt_AttrSetting,			"Control Attribute Info"},
	{_tag_POS_SIZE,				g_Prpt_PosSize,				"Control Position and Size Info"},
	{_tag_LIST,					g_Prpt_ListCtrl,			"Control List Infomation"},
	{_tag_TEXT,					g_Prpt_TextInfoType2,		"Text Infomation"},
	{_tag_IMGMODE,				g_Prpt_ImageMode,			"Control Image Mode"},
	{_tag_UVMODE,				g_Prpt_ImageUVMode,			"Control Image UV Mode"},
	//{_tag_BLOCKMODE,			g_Prpt_ImageBlockMode,		"Control Image Block Mode"},

	{NULL},
};

ParameterDefine g_Param_Text[] = 
{
	{_tag_MISC,					g_Prpt_Misc,				"Control Misc Infomation"},
	{_tag_ATTR,					g_Prpt_AttrSetting,			"Control Attribute Info"},
	{_tag_POS_SIZE,				g_Prpt_PosSize,				"Control Position and Size Info"},
	{_tag_TEXT,					g_Prpt_TextInfoType3,		"Text Infomation"},

	{NULL},
};


ParameterDefine g_Param_ImageCtrl[] = 
{
	{_tag_MISC,					g_Prpt_Misc,				"Control Misc Infomation"},
	{_tag_ATTR,					g_Prpt_AttrSetting,			"Control Attribute Info"},
	{_tag_POS_SIZE,				g_Prpt_PosSize,				"Control Position and Size Info"},
	{_tag_IMGMODE,				g_Prpt_ImageMode,			"Control Image Mode"},
	{_tag_UV_BLOCKMODE,			g_Prpt_ImageImgCtrl,		"Control Image Mode"},
	{_tag_FILEMODE,				g_Prpt_ImageFile,			"Load Image File"},

	{NULL},
};

ParameterDefine g_Param_Slider[] = 
{
	{_tag_MISC,					g_Prpt_Misc,				"Control Misc Infomation"},
	{_tag_ATTR,					g_Prpt_AttrSetting,			"Control Attribute Info"},
	{_tag_POS_SIZE,				g_Prpt_PosSize,				"Control Position and Size Info"},
	{_tag_SLIDER,				g_Prpt_Slider,				"Slider Infomation"},
	{_tag_TEXT,					g_Prpt_TextInfoType2,		"Text Infomation"},
	{_tag_IMGMODE,				g_Prpt_ImageMode,			"Control Image Mode"},
	{_tag_SD_NMR_UVMODE,		g_Prpt_ImageSlideNormal,	"Slider Normal UV Mode"},
	{_tag_SD_GAUGE_UVMODE,		g_Prpt_ImageSlideGauge,		"Slider Gauge UV Mode"},
	{_tag_SD_ETC_UVMODE,		g_Prpt_ImageSlideEtc,		"Slider Etc UV Mode"},

	{NULL},
};



//-----------------------------------------------------------------------------------------
PropertyDefine g_Prpt_EditingFontMisc[] = 
{
	{"Attribute Name",			_CPT_S_ATTR_NAME,			_VT_CHAR,	"Font Attribute List and Add/Remove", TRUE},
	{"Enable Edit",				_CPT_B_ATTR_EDIT,			_VT_BOOL,	"Enable Edit", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_EditingFontAttr[] = 
{	
	{"Reset",					_CPT_B_ATTR_RESET,			_VT_BOOL,	"Reset", TRUE},
	{"Type",					_CPT_E_ATTR_TYPE,			_VT_ENUM,	"Font Type", TRUE},
	{"Color RGB",				_CPT_CLR_ATTR_COLOR_RGB,	_VT_COLOR,	"color Red, Green, Blue (max : 255)", TRUE},
	{"Color Alpha",				_CPT_I_ATTR_COLOR_A,		_VT_INT,	"color Alpha (max : 255)", TRUE},
	{"Width Scale",				_CPT_I_ATTR_SCALE_WIDTH,	_VT_INT,	"Width Scale(%)", TRUE},
	{"Hieght Scale",			_CPT_I_ATTR_SCALE_HIEGHT,	_VT_INT,	"Hieght Scale(%)", TRUE},
	{"Style",					_CPT_E_ATTR_STYLE,			_VT_ENUM,	"Regular / Italic", TRUE},
	{"Align",					_CPT_E_ATTR_ALIGN,			_VT_ENUM,	"Align - left/center/right", TRUE},
	{"Word space",				_CPT_I_ATTR_CHAR_SPACE,		_VT_INT,	"Word space", TRUE},
	{"Line space",				_CPT_I_ATTR_LINE_SPACE,		_VT_INT,	"Line space", TRUE},
	{"Word Wrap",				_CPT_B_ATTR_WRAP,			_VT_BOOL,	"Word Wrap", TRUE},
	{"Truncate",				_CPT_B_ATTR_TRUNCATE,		_VT_BOOL,	"Truncate", TRUE},
	{"Ignore",					_CPT_B_ATTR_IGNORE,			_VT_BOOL,	"Ignore", TRUE},
	
	{NULL},
};

ParameterDefine g_Param_EditingFontAttr[] = 
{
	{"Misc",			g_Prpt_EditingFontMisc, "Font Misc"},
	{"Font Attribute",	g_Prpt_EditingFontAttr, "Font Attribute Infomation"},

	{NULL},
};

//-----------------------------------------------------------------------------------------
PropertyDefine g_Prpt_Selected_Pos_N_SizeSetting[] = 
{
	{"Add Pos X",		_CPT_I_ADD_X,			_VT_INT,	"Selected Controls Add Pos X", TRUE},
	{"Add Pos Y",		_CPT_I_ADD_Y,			_VT_INT,	"Selected Controls Add Pos Y", TRUE},
	{"Add Width",		_CPT_I_ADD_WIDTH,		_VT_INT,	"Selected Controls Add Width", TRUE},
	{"Add Height",		_CPT_I_ADD_HEIGHT,		_VT_INT,	"Selected Controls Add Height", TRUE},

	{NULL},
};

ParameterDefine g_Param_Selected[] = 
{
	{_tag_ATTR,					g_Prpt_AttrSetting,			"Control Attribute Info"},
	{"Selected Control Position and Size", g_Prpt_Selected_Pos_N_SizeSetting,  "Selected Control Position and Size Info"},

	{NULL},
};

//-----------------------------------------------------------------------------------------
PropertyDefine g_Prpt_EditingTexId[] = 
{
	{"Name",			_CPT_S_TEX_UNIT_NAME,	_VT_CHAR,	"Textrue Name", TRUE},
	{"ID",				_CPT_S_TEX_UNIT_ID,		_VT_CHAR,	"Texture ID", FALSE},
	{"File Name",		_CPT_S_TEX_FILE,		_VT_CHAR,	"Texture File Name", FALSE},
	{"Local",			_CPT_S_TEX_LOCAL,		_VT_CHAR,	"Localized Texture", FALSE},

	{NULL},
};

PropertyDefine g_Prpt_EditingUVId[] = 
{
	{"Name",			_CPT_S_UV_NAME,			_VT_CHAR,	"UV Name", TRUE},
	{"ID",				_CPT_S_UV_ID,			_VT_CHAR,	"UV ID", FALSE},

	{NULL},
};

PropertyDefine g_Prpt_EditingUVCoordinates[] = 
{
	{"U1",				_CPT_I_UV_U1,			_VT_INT,	"U1 Coordinates", TRUE},
	{"V1",				_CPT_I_UV_V1,			_VT_INT,	"V1 Coordinates", TRUE},
	{"U2",				_CPT_I_UV_U2,			_VT_INT,	"U2 Coordinates", TRUE},
	{"V2",				_CPT_I_UV_V2,			_VT_INT,	"V2 Coordinates", TRUE},
	{"Width",			_CPT_I_UV_WIDTH,		_VT_INT,	"UV Width", TRUE},
	{"Height",			_CPT_I_UV_HEIGHT,		_VT_INT,	"UV Height", TRUE},

	{NULL},
};

PropertyDefine g_Prpt_EditingImgCondition[] = 
{
	{"Show UV Block",	_CPT_B_UV_SHOW_BLOCK,	_VT_BOOL,	"Show UV Block On/Off", TRUE},
	{"Show UV Id",		_CPT_B_UV_SHOW_ID,		_VT_BOOL,	"Show UV Index On/Off", TRUE},
	{NULL},
};

ParameterDefine g_Param_EditingImage[] = 
{	
	{"UV Id",			g_Prpt_EditingUVId, "UV Id Info"},
	{"UV Coordinates",	g_Prpt_EditingUVCoordinates, "UV Coordinates Info"},
	{"Texture Id",		g_Prpt_EditingTexId, "Texture Id Info"},
	{"Edit Condition",	g_Prpt_EditingImgCondition, "Edit Condition"},

	{NULL},
};

ParameterDefine g_Param_EditingImageCondition[] = 
{
	{"Texture Id",		g_Prpt_EditingTexId, "Texture Id Info"},
	{"Edit Condition", g_Prpt_EditingImgCondition, "Edit Condition"},

	{NULL},
};
