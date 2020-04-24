#pragma once

namespace pvz {
#pragma pack(push,1)
	struct info
	{
		int offset;
		int max_num;
		int upper_bound;
		int next_index;
		int current_num;
		int last_index;
	};
	struct zombie
	{
		int unused1[2];
		int x;
		int y;		//10
		int unused2[3];
		int row;	//20
		int unused3;
		int kind;
		int state;	//2C
		int unused4[15];
		int countdown;//6C
		int unused5[32];
		int disappeared;//F0
		int unused6[27];//15C
	};
	struct plant
	{
		int unused1[7];	//1c
		int row;		//20
		int unused2;	//24
		int kind;		//28
		int col;		//2c
		int unused3[4];	//3c
		int state;		//40
		int hp;			//44
		int max_hp;		//48
		char unused4[249];//141 unused4[240] 是模仿对象
		short disapeared_or_crushed;//143
		char unused5[5];//14c
		int index;
	};
	struct item
	{
		int unused1[9];	//24
		float x;
		float y;		//2c
		int unused2[3];	//38
		int disapeared;
		int disapeared1;//40
		int unused3[4];	//50
		int collected;	//54
		int unused4[33];//d8
	};
	struct slot
	{
		int unused1[9];	//24
		int cooldown;
		int	total_cd;
		int unused2[2];
		int kind;
		int imitater_kind;//40
		int unused3[5];	//50
	};
#pragma pack(pop)
}