#pragma		 once

#define MAX_TROOP_AI_ID		-800000


enum TAI_IF
{
	TAI_IF_START= -900000,
	IfHP,
	IfMeleeEngaged,
	IfEngaged,
	IfNumFriendlyTroop,
	IfNumEnemyTroop,
	IfThereIsTroop,
	IfIAm,
	TAI_IF_LAST
};

enum TAI_TYPE
{
	TAI_TYPE_START	=	-900500,
	Engage,
	EngageMercifully,
	EngageSmart,
	EngageStanding,
	EngageStandingMercifully,
	Disengage,
	Defense,
	DefenseProp,
	UseAbility,
	MoveOnPath,
	MoveOnPathLoop,
	MoveOnPathRandom,
	MoveOnPathWithAvoidEnemy,
	MoveOnFullPath,
	AttackEnemyNearSomebody,
	AttackWall,
	SetTargetID,
	Stop,
	Follow,
	EngageGuardianToTarget,
	EngageArea,
	DisengageOnAttacked,
	EngageMercifullyArea,
	TAI_TYPE_LAST
};


enum TAI_FILTER
{
	TAI_FILTER_START,

	Friend,
	Mine,
	Partner,
	Enemy,
	
	Player,
	Player1,
	Player2,
	MyPlayer,
	OtherPlayer,
	MyLinker,

	Ground,
	
	Guardian,
	Guardian1,
	Guardian2,

	Melee,
	Sword,
	Spear,

	Range,
	Archer,

	Siege,
	Ballista,
	Seiran,

	Boss,

	Flying,
	Dragon,

	People,

	// Special

	ByID,

	RealTroopToHere,

	AIGuide0,
	AIGuide1,
	AIGuide2,
	AIGuide3,
	AIGuide4,
	AIGuide5,
	AIGuide6,
	AIGuide7,
	AIGuide8,
	AIGuide9,
	AIGuide10,
	AIGuide11,
	AIGuide12,
	AIGuide13,
	AIGuide14,
	AIGuide15,

	Adjective,	//----------

	HPUnder25,
	HPUnder50,
	HPUnder75,

	MyWorst,
	MyPlayersWorst,
	OtherPlayersWorst,
	SomebodysWorst,
	SwordsWorst,
	SpearsWorst,
	MeleesWorst,
	ArchersWorst,
	BallistasWorst,
	RangesWorst,

	RideHorse,
	Wall,

	Superlative,	//----------

	NearestToPlayer1,
	NearestToPlayer2,
	NearestToMyPlayer,
	NearestToOtherPlayer,
	Nearest,

	Weakest,
	Strongest,

	FarthestToPlayer1,
	FarthestToPlayer2,
	FarthestToMyPlayer,
	FarthestToOtherPlayer,
	Farthest,
	
	TAI_FILTER_LAST 
    //데이터를 하나 추가하셨으면 아래 함수에 적합한 형태로 추가해주세요.
    //툴에서 문자열을 가져오기 위해 꼭 필요함
};

enum TAI_PARAM
{
	Walk,
	Run,
	Forward,
	Backward,

	// follow type
	Front,
	Gather,
	Tail,

	PlayerType,
	GroundType,
	GuardianType,
	MeleeType,
	SwordType,
	SpearType,
	RangeType,
	ArcherType,
	SiegeType,
	BallistaType,
	SeiranType,
	BossType,
	FlyingType,
	DragonType
};

