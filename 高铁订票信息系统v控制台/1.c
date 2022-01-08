#include<stdio.h>
#include<string.h>
#include<time.h>
#include<windows.h>
#include<stdlib.h>

#include"menu.h"

#pragma warning(disable:6011)
#pragma warning(disable:6387)
#pragma warning(disable:6031)
#pragma warning(disable:28182)

//高铁信息 存储结构
struct Route
{
	int id_Route;
	char Num_Train[7];//车编号 用于辨别车次
	char Station_Begin[10];//起点站
	char Station_End[10];//到达站

	long int date_Begin;//出发时间
	int time_Begin;//出发时
	int second_Begin;//出发分

	int date_End;//到站日期
	int time_End;//到站时
	int second_End;//到站分

	int Level[3];// 0 1 2 分别是特等 一等 二等
	int SumPeople; //人员定额
	int Booked;//已购票人数
	int Remain;//剩余票数
	int Wait;//候补人数

	struct Route* next;
};

//用户信息结构
struct user
{
	long long int Id;//身份证
	char Name[20];//姓名

	char Num_Train[7];//车编号 用于辨别车次
	char Station_Begin[10];//起点站
	char Station_End[10];//到达站

	long int date_Begin;//出发时间
	int time_Begin;//出发时
	int second_Begin;//出发分

	int Level;//席别
	int ID_Route;//车编号 用于辨别车次

	struct user* next;
};

//头节点
struct Route* head_Route;//路线 头节点
struct user* head_Booked;//已购票人 头节点
struct user* head_Wait;//待候补 头节点
struct user* head_Waited;//候补成功

//文件指针
FILE* fpRoute;
FILE* fpBookedPeo;
FILE* fpWaitPeo;
FILE* fpWaitedPeo;
FILE* fptemp;


////////////////////////////////////////////////////////////////////////////////////////////////初始化函数/////////
//判断是否找到数据文件 包括：路线信息 已订票名单 候补名单 ----- 找到返回1 失败返回0
void Find_Database(FILE* fpRoute, FILE* fpBookedPeo, FILE* fpWaitPeo, FILE* fpWaitedPeo);
//读取 Route.txt 载入链表
void InitList_Route(FILE* fpRoute);
//释放旧RoteLists
void FreeList_Route(struct Route* head);
//读取 BookedPeo.txt 载入链表
void InitList_BookedPeo(FILE* fpBookedPeo);
//读取 候补名单文件 载入链表
void InitList_WaitPeo();
//读取 候补成功名单文件 载入链表
void InitList_waitedPeo();
//释放User相关信息链表
void FreeList_User(struct user* head);
//更新Route.txt 
void Update_FileRoute(struct Route* head_Route1);
//更新user相关文件 参数：待更新的文件链表头  文件名地址
void Update_FileUser(struct user* head, char* Filename);
//更新waited 的 链表 和 文件
void Update_Waited(struct user* head_Waited, long long int ID);
//更新Booked 的 链表 和 文件
void Update_Booked(struct user* head_Booked, long long int ID);
//向后写入BookedPeo
void Write_Booked(struct user* head_Bookedpeo);

/////////////////////////////////////////////////////////////////////////////////////////////////////主要函数///////
//自动候补购票
void AutoBooked_ticket(long long int ID, int id_route, int level, char* name);
//找是否 有能候补的乘客
void Find_WaitPeo(int Idroute, int level);
//是否选择候补  成功候补返回 1
int Select_wait(int idroute, int level, struct Route* now);
// 查询车次
void Search_Route();
//订车票
void Book_Ticket();
//退订车票
void Cancel_Ticket();
//个人查票
void Search_Inf();
//候补情况查询
void Search_wait();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main()
{
	//判断文件是否存在
	Find_Database(fpRoute, fpBookedPeo, fpWaitPeo, fpWaitedPeo);

	//菜单功能
	int select = 0;
	UserMenu();

	while (1)
	{
		if (select != 9)
		{
			printf("请输入指令：");
		}
		scanf_s("%d", &select);
		switch (select)
		{
		case 1:
			// 1.查询车票
			Search_Route();
			break;
		case 2:
			//2.预订车票
			Book_Ticket();
			break;
		case 3:
			//3.退订车票
			Cancel_Ticket();
			break;
		case 4:
			//4.候补查询
			Search_wait();
			break;
		case 5:
			Search_Inf();
			break;
		case 9:
			system("CLS");
			AdminMenu(); //9.后台管理
			break;
		case 0:   //0.退出程序
			printf("\n欢迎下次使用\n");
			exit(0);
			break;

		default:
			printf("请重新输入\n");
			Sleep(800);
			system("CLS");
			UserMenu();
			break;
		}
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//判断是否找到数据文件 包括：路线信息 已订票名单 候补名单 ----- 找到返回1 失败返回0
void Find_Database(FILE* fpRoute, FILE* fpBookedPeo, FILE* fpWaitPeo,FILE* fpWaitedPeo)
{
	//fopen_s 成功返回0 不成功返回其他数值
	int a = fopen_s(&fpBookedPeo, "BookedPeo.txt", "r");
	int b = fopen_s(&fpWaitPeo, "WaitPeo.txt", "r");
	int c = fopen_s(&fpRoute, "Route.txt", "r");
	int d = fopen_s(&fpWaitedPeo, "WaitedPeo.txt", "r");

	if (a == 0 && b == 0 && c == 0 && d== 0)
	{
		printf("…………数据文件已找到……………\n\n");
		fclose(fpRoute);
		fclose(fpBookedPeo);
		fclose(fpWaitPeo);
		fclose(fpWaitedPeo);
	
	}
	else//文件丢失
	{
		printf("数据文件未找到\n");
		fclose(fpRoute);
		fclose(fpBookedPeo);
		fclose(fpWaitPeo);
		fclose(fpWaitedPeo);

		exit(0);
	}
}

//读取 Route.txt 载入链表
void InitList_Route(FILE* fpRoute)
{
	fopen_s(&fpRoute, "Route.txt", "r");// fpRoute 指向Route.txt的文件流

	if (!feof(fpRoute))
	{	//建立头节点
		head_Route = (struct Route*)malloc(sizeof(struct Route));
		head_Route->next = NULL;

		//建立临时节点 用于刷新
		struct Route* temp;
		temp = head_Route;

		char ch;


		while (!feof(fpRoute))
		{
			fscanf_s(fpRoute, "%d", &temp->id_Route);//读入路线id

			fscanf_s(fpRoute, "%s", temp->Num_Train, 7);//读入车次编号

			fscanf_s(fpRoute, "%s", temp->Station_Begin, 10);//读入起点站

			fscanf_s(fpRoute, "%s", temp->Station_End, 10);//读入终点站

			fscanf_s(fpRoute, "%ld", &temp->date_Begin);//读入开始日期

			fscanf_s(fpRoute, "%d", &temp->time_Begin);//读入 开始时

			fscanf_s(fpRoute, "%d", &temp->second_Begin);//读入 开始分

			fscanf_s(fpRoute, "%ld", &temp->date_End);//读入终点日期

			fscanf_s(fpRoute, "%d", &temp->time_End);//读入 终点时

			fscanf_s(fpRoute, "%d", &temp->second_End);//读入 终点分

			fscanf_s(fpRoute, "%d %d %d", &temp->Level[0], &temp->Level[1], &temp->Level[2]);//读入 特等座 一等座 二等座 数量

			fscanf_s(fpRoute, "%d %d %d %d", &temp->SumPeople, &temp->Booked, &temp->Remain, &temp->Wait);//读入 成员定额 已购票人数 剩余票数 候补人数

			ch = fgetc(fpRoute);//吞掉 \n

			struct Route* Node = (struct Route*)malloc(sizeof(struct Route));

			Node->next = NULL;

			temp->next = Node;
			temp = Node;

		}
		temp = NULL;
		free(temp);

	}


	fclose(fpRoute);

}

//释放旧RoteLists
void FreeList_Route(struct Route* head)
{
	struct Route* pre, * p;
	pre = head;
	p = pre->next;

	while (p != NULL)
	{
		free(pre);
		pre = p;
		p = p->next;
	}
	free(pre);

}

//读取 BookedPeo.txt 载入链表
void InitList_BookedPeo(FILE* fpBookedPeo)
{
	fopen_s(&fpBookedPeo, "BookedPeo.txt", "r");

	if (!feof(fpBookedPeo))//非空文件
	{
		//载入
		head_Booked = (struct user*)malloc(sizeof(struct user));
		head_Booked->next = NULL;

		struct user* temp_Booked;
		temp_Booked = head_Booked;//只用于记录当前位置 头开始

		if (temp_Booked != NULL && head_Booked != NULL)
		{
			while (!feof(fpBookedPeo))
			{
				fscanf_s(fpBookedPeo, "%lld", &temp_Booked->Id);//读身份证
				fscanf_s(fpBookedPeo, "%s", temp_Booked->Name, 20);//读取名字
				fscanf_s(fpBookedPeo, "%s", temp_Booked->Num_Train, 7);//读取名字
				fscanf_s(fpBookedPeo, "%s", temp_Booked->Station_Begin, 10);//读起点站
				fscanf_s(fpBookedPeo, "%s", temp_Booked->Station_End, 10);//读终点站
				fscanf_s(fpBookedPeo, "%ld", &temp_Booked->date_Begin);//读日期
				fscanf_s(fpBookedPeo, "%d", &temp_Booked->time_Begin);//读时
				fscanf_s(fpBookedPeo, "%d", &temp_Booked->second_Begin);//读分
				fscanf_s(fpBookedPeo, "%d", &temp_Booked->Level);//读席别
				fscanf_s(fpBookedPeo, "%d", &temp_Booked->ID_Route);//读路线ID

				struct user* Node = (struct user*)malloc(sizeof(struct user));
				Node->next = NULL;
				temp_Booked->next = Node;
				temp_Booked = Node;
			}

		}
		temp_Booked = NULL;
		free(temp_Booked);
	}

	//关闭文件
	fclose(fpBookedPeo);

}

//读取 候补名单文件 载入链表
void InitList_WaitPeo()
{
	fopen_s(&fpWaitPeo,"WaitPeo.txt","r");

	if (!feof(fpWaitPeo))//非空文件
	{
		head_Wait = (struct user*)malloc(sizeof(struct user));
		struct user* temp_Wait = head_Wait;


		while (!feof(fpWaitPeo))
		{
			fscanf_s(fpWaitPeo, "%lld", &temp_Wait->Id);//读身份证
			fscanf_s(fpWaitPeo, "%s", temp_Wait->Name, 20);//读取名字
			fscanf_s(fpWaitPeo, "%s", temp_Wait->Num_Train, 7);//读取车编号
			fscanf_s(fpWaitPeo, "%s", temp_Wait->Station_Begin, 10);//读起点站
			fscanf_s(fpWaitPeo, "%s", temp_Wait->Station_End, 10);//读终点站
			fscanf_s(fpWaitPeo, "%ld", &temp_Wait->date_Begin);//读日期
			fscanf_s(fpWaitPeo, "%d", &temp_Wait->time_Begin);//读时
			fscanf_s(fpWaitPeo, "%d", &temp_Wait->second_Begin);//读分
			fscanf_s(fpWaitPeo, "%d", &temp_Wait->Level);//读席别
			fscanf_s(fpWaitPeo, "%d", &temp_Wait->ID_Route);//读路线ID

			struct user* Node = (struct user*)malloc(sizeof(struct user));
			Node->next = NULL;
			temp_Wait->next = Node;
			temp_Wait = Node;

		}
		temp_Wait = NULL;
		free(temp_Wait);
	}

	fclose(fpWaitPeo);
}

//读取 候补成功名单文件 载入链表
void InitList_waitedPeo()
{
	fopen_s(&fpWaitedPeo, "WaitedPeo.txt", "r");

	if (!feof(fpWaitedPeo))//非空文件
	{
		head_Waited = (struct user*)malloc(sizeof(struct user));
		struct user* temp_Waited_temp = head_Waited;


		while (!feof(fpWaitedPeo))
		{
			fscanf_s(fpWaitedPeo, "%lld", &temp_Waited_temp->Id);//读身份证
			fscanf_s(fpWaitedPeo, "%s", temp_Waited_temp->Name, 20);//读取名字
			fscanf_s(fpWaitedPeo, "%s", temp_Waited_temp->Num_Train, 7);//读取车编号
			fscanf_s(fpWaitedPeo, "%s", temp_Waited_temp->Station_Begin, 10);//读起点站
			fscanf_s(fpWaitedPeo, "%s", temp_Waited_temp->Station_End, 10);//读终点站
			fscanf_s(fpWaitedPeo, "%ld", &temp_Waited_temp->date_Begin);//读日期
			fscanf_s(fpWaitedPeo, "%d", &temp_Waited_temp->time_Begin);//读时
			fscanf_s(fpWaitedPeo, "%d", &temp_Waited_temp->second_Begin);//读分
			fscanf_s(fpWaitedPeo, "%d", &temp_Waited_temp->Level);//读席别
			fscanf_s(fpWaitedPeo, "%d", &temp_Waited_temp->ID_Route);//读路线ID

			struct user* Node = (struct user*)malloc(sizeof(struct user));
			Node->next = NULL;
			temp_Waited_temp->next = Node;
			temp_Waited_temp = Node;

		}
		temp_Waited_temp = NULL;
		free(temp_Waited_temp);
	}

	fclose(fpWaitedPeo);

}

//释放User相关信息链表
void FreeList_User(struct user* head)
{
	struct user* pre, * p;
	pre = head;
	p = pre->next;

	if (head == NULL)
	{
		return;
	}

	while (p != NULL)
	{
		free(pre);
		pre = p;
		p = p->next;
	}

	free(pre);
}

///////////////////////////////////////////////////////////////////////////功能区////////////////////////////////////////////

//更新Route.txt 
void Update_FileRoute(struct Route* head_Route1)
{

	fopen_s(&fptemp, "temp.txt", "w"); // 创建新文件
	struct Route* temp = head_Route1;

	while (temp->next->next != NULL)
	{
		fprintf_s(fptemp, "%d ", temp->id_Route);
		fprintf_s(fptemp, "%s ", temp->Num_Train);
		fprintf_s(fptemp, "%s ", temp->Station_Begin);
		fprintf_s(fptemp, "%s ", temp->Station_End);
		fprintf_s(fptemp, "%ld ", temp->date_Begin);
		fprintf_s(fptemp, "%2d ", temp->time_Begin);
		fprintf_s(fptemp, "%2d ", temp->second_Begin);
		fprintf_s(fptemp, "%ld ", temp->date_End);
		fprintf_s(fptemp, "%2d ", temp->time_End);
		fprintf_s(fptemp, "%2d ", temp->second_End);
		fprintf_s(fptemp, "%2d ", temp->Level[0]);
		fprintf_s(fptemp, "%2d ", temp->Level[1]);
		fprintf_s(fptemp, "%2d ", temp->Level[2]);
		fprintf_s(fptemp, "%2d ", temp->SumPeople);
		fprintf_s(fptemp, "%2d ", temp->Booked);
		fprintf_s(fptemp, "%2d ", temp->Remain);
		fprintf_s(fptemp, "%2d", temp->Wait);
		fprintf_s(fptemp, "\n");
		temp = temp->next;
	}

	fclose(fptemp);
	remove("Route.txt");
	rename("temp.txt", "Route.txt");


}

//更新user相关文件 参数：待更新的文件链表头  文件名地址
void Update_FileUser(struct user* head ,char* Filename)
{
	fopen_s(&fptemp, "temp.txt", "w"); // 创建新文件
	struct user* temp = head;


	while (temp->next->next != NULL)
	{
		fprintf_s(fptemp, "\n");
		fprintf_s(fptemp, "%lld ", temp->Id);
		fprintf_s(fptemp, "%s ", temp->Name);
		fprintf_s(fptemp, "%s ", temp->Num_Train);
		fprintf_s(fptemp, "%s ", temp->Station_Begin);
		fprintf_s(fptemp, "%s ", temp->Station_End);
		fprintf_s(fptemp, "%d ", temp->date_Begin);
		fprintf_s(fptemp, "%d ", temp->time_Begin);
		fprintf_s(fptemp, "%d ", temp->second_Begin);
		fprintf_s(fptemp, "%d ", temp->Level);
		fprintf_s(fptemp, "%d", temp->ID_Route);
		temp = temp->next;
	}

	fclose(fptemp);
	remove(Filename);
	rename("temp.txt", Filename);
}

//更新waited 的 链表 和 文件
void Update_Waited(struct user* head_Waited, long long int ID)
{
	struct user* temp_Waited = head_Waited;//当前
	struct user* temp_front = head_Waited;//上一个

	if (temp_Waited == NULL)
	{
		return;
	}

	//查 删 ，同时记录两个节点
	while (temp_Waited->next != NULL)
	{

		if (ID == temp_Waited->Id) // 找到该节点
		{

			if (temp_Waited == head_Waited)//删除头节点
			{
				//第一条为测试数据，不能删除，可以忽略此情况
			}
			else//删除 非头节点
			{
				temp_front->next = temp_Waited->next;
				free(temp_Waited);
			}
			break;
		}
		temp_front = temp_Waited;
		temp_Waited = temp_Waited->next;
	}
	//删除最后一个节点
	if (ID == temp_Waited->Id)
	{
		temp_front->next = NULL;
		free(temp_Waited);//释放
	}

	//将新的 List 写入更新WaitedPeo.txt
	Update_FileUser(head_Waited, "WaitedPeo.txt");


}

//更新Booked 的 链表 和 文件
void Update_Booked(struct user* head_Booked, long long int ID)
{
	struct user* temp_Booked = head_Booked;//当前
	struct user* temp_front = head_Booked;//上一个

	if (temp_Booked == NULL)
	{
		return;
	}

	//查 删 ，同时记录两个节点
	while (temp_Booked->next != NULL)
	{

		if (ID == temp_Booked->Id) // 找到该节点
		{

			if (temp_Booked == head_Waited)//删除头节点
			{
				//第一条为测试数据，不能删除，可以忽略此情况
			}
			else//删除 非头节点
			{
				temp_front->next = temp_Booked->next;
				free(temp_Booked);
			}
			break;
		}
		temp_front = temp_Booked;
		temp_Booked = temp_Booked->next;
	}
	//删除最后一个节点
	if (ID == temp_Booked->Id)
	{
		temp_front->next = NULL;
		free(temp_Booked);//释放
	}

	//更新文件
	Update_FileUser(head_Booked, "BookedPeo.txt");

}

//向后写入BookedPeo
void Write_Booked(struct user* head_Bookedpeo)
{
	fopen_s(&fpBookedPeo, "BookedPeo.txt", "a");//指向文件尾 

	fprintf_s(fpBookedPeo, "\n");

	fprintf_s(fpBookedPeo, "%lld", head_Booked->Id);//购票人身份证号码
	fprintf_s(fpBookedPeo, " ");

	fputs(head_Booked->Name, fpBookedPeo);//购票人姓名
	fprintf_s(fpBookedPeo, " ");

	fputs(head_Booked->Num_Train,fpBookedPeo);//车号码
	fprintf_s(fpBookedPeo, " ");

	fputs(head_Booked->Station_Begin, fpBookedPeo);//起点站
	fprintf_s(fpBookedPeo, " ");

	fputs(head_Booked->Station_End, fpBookedPeo);//终点站
	fprintf_s(fpBookedPeo, " ");

	fprintf_s(fpBookedPeo, "%ld", head_Booked->date_Begin);//日期
	fprintf_s(fpBookedPeo, " ");

	fprintf_s(fpBookedPeo, "%d", head_Booked->time_Begin);//时
	fprintf_s(fpBookedPeo, " ");

	fprintf_s(fpBookedPeo, "%d", head_Booked->second_Begin);//分
	fprintf_s(fpBookedPeo, " ");

	fprintf_s(fpBookedPeo, "%d", head_Booked->Level);//席别
	fprintf_s(fpBookedPeo, " ");
	fprintf_s(fpBookedPeo, "%d", head_Booked->ID_Route);//车id

	//关闭
	fclose(fpBookedPeo);

	free(head_Booked);//释放


}

//自动候补购票
void AutoBooked_ticket(long long int ID, int id_route, int level, char* name)
{
	struct Route* temp = head_Route;
	//temp 找到指定路线
	while (temp != NULL)
	{
		if (id_route = temp->id_Route )//找到
		{
			break;
		}
		temp = temp->next;
	}

	// 购票人节点  信息输入
	free(head_Booked);
	head_Booked = (struct user*)malloc(sizeof(struct user));
	head_Booked->Level = level;//席别
	strcpy_s(head_Booked->Name, 20 ,name);//姓名
	head_Booked->Id = ID;//身份证号
	head_Booked->ID_Route = id_route;//路线ID
	strcpy_s(head_Booked->Station_Begin, 10, temp->Station_Begin);//起点站	
	strcpy_s(head_Booked->Station_End, 10, temp->Station_End);//终点站
	head_Booked->date_Begin = temp->date_Begin;//日期
	head_Booked->time_Begin = temp->time_Begin;//时
	head_Booked->second_Begin = temp->second_Begin;//分
	strcpy_s(head_Booked->Num_Train, 7, temp->Num_Train);//起点站	

	//刷新routelist
	if (level == 0)
	{
		temp->Level[0]--;//特等座减一
		temp->Remain--;
		temp->Booked++;
	}
	else if (level == 1)
	{
		temp->Level[1]--;//特等座减一
		temp->Remain--;
		temp->Booked++;
	}
	else if (level == 2)
	{
		temp->Level[2]--;//特等座减一
		temp->Remain--;
		temp->Booked++;
	}
	temp->Wait--;

	//更新 Route.text 文件  覆盖
	Update_FileRoute(head_Route);

	//写入 BookedPeo.txt
	Write_Booked(head_Booked);

}

//找是否 有能候补的乘客
void Find_WaitPeo( int Idroute,int level)
{
	InitList_WaitPeo();
	struct user* temp_FindWait = head_Wait;
	_Bool findwait = FALSE;

	if (temp_FindWait == NULL)
	{
		return;
	}

	while (temp_FindWait->next != NULL)
	{
		if (Idroute == temp_FindWait->ID_Route && level == temp_FindWait->Level)//符合候补条件:路线 + 席别
		{
			findwait = TRUE;

			//自动候补买票
			AutoBooked_ticket(temp_FindWait->Id,temp_FindWait->ID_Route,temp_FindWait->Level,temp_FindWait->Name);
			printf("\n系统已经为姓名：%s 的用户成功后补一张车票\n", temp_FindWait->Name);
			Line();



			//*** Waitpeo更新
			InitList_WaitPeo();
			//删去 并写入
			struct user* temp_del_waitpeo = head_Wait;
			struct user* temp_front = head_Wait;
			_Bool delete = FALSE;

			while (temp_del_waitpeo->next != NULL)
			{

				//有第一行是测试数据，所以不会是头节点
				if (temp_del_waitpeo->Id == temp_FindWait->Id && temp_del_waitpeo->ID_Route == temp_FindWait->ID_Route)//找到这个人
				{

					delete = TRUE;		

					temp_front->next = temp_del_waitpeo->next;
					free(temp_del_waitpeo);
					break;
				}

				temp_front = temp_del_waitpeo;
				temp_del_waitpeo = temp_del_waitpeo->next;
			}

			//最后一个节点
			if (delete == FALSE)
			{
				if (temp_del_waitpeo->Id == temp_FindWait->Id && temp_del_waitpeo->ID_Route == temp_FindWait->ID_Route)//找到这个人
				{

					_Bool delete = TRUE;
					temp_front->next = NULL;
					free(temp_del_waitpeo);

				}
			}

			//写入文件
			Update_FileUser(head_Wait,"WaitPeo.txt");


			//WaitedPeo更新 向后加入
			fopen_s(&fpWaitedPeo,"WaitedPeo.txt","a");
			fprintf_s(fpWaitedPeo, "\n");//换行
			fprintf_s(fpWaitedPeo,"%lld ",temp_FindWait->Id);//身份证号
			fprintf_s(fpWaitedPeo, "%s ", temp_FindWait->Name);//名字
			fprintf_s(fpWaitedPeo, "%s ", temp_FindWait->Num_Train);//车编号
			fprintf_s(fpWaitedPeo, "%s ", temp_FindWait->Station_Begin);//起点站
			fprintf_s(fpWaitedPeo, "%s ", temp_FindWait->Station_End);//终点站
			fprintf_s(fpWaitedPeo, "%ld ", temp_FindWait->date_Begin);//日期
			fprintf_s(fpWaitedPeo, "%d ", temp_FindWait->time_Begin);//时
			fprintf_s(fpWaitedPeo, "%d ", temp_FindWait->second_Begin);//分
			fprintf_s(fpWaitedPeo, "%d ", temp_FindWait->Level);//席别
			fprintf_s(fpWaitedPeo, "%d", temp_FindWait->ID_Route);//ID-Route
			fclose(fpWaitedPeo);

			return;
		}
		temp_FindWait = temp_FindWait->next;
	}


	if (findwait == FALSE)//最后一个节点前 仍未找到
	{
		if (Idroute == temp_FindWait->ID_Route && level == temp_FindWait->Level)//符合候补条件
		{
			findwait = TRUE;
			//自动候补
			AutoBooked_ticket(temp_FindWait->Id, temp_FindWait->ID_Route, temp_FindWait->Level, temp_FindWait->Name);
			printf("/n有一位待候补乘客已经成功候补\n姓名为：%s", temp_FindWait->Name);
			
		}
	}


	//找不到合适的
	if (findwait == FALSE)
	{
		printf("无需要自动候补的成员\n");
		Line();

		return;
	}

}

//是否选择候补  成功候补返回 1
int Select_wait( int idroute , int level, struct Route* now)
{
	long long ID;
	char name[20];
	Line();
	printf("您选择了候补\n请输入你的身份证号码：");
	scanf_s("%lld",&ID);

	//先判断是否已经买过该车次的车票
	InitList_BookedPeo(fpBookedPeo);
	struct user* temp_if_again = head_Booked;
	long long int tempid = ID;
	while (temp_if_again != NULL)
	{
		if (temp_if_again->Id == tempid)
		{
			Line();
			printf("每个证件号最多只能候补一张同一车次的车票！请勿重复添加候补！");
			Line();
			return;
			free(head_Booked);
		}
		temp_if_again = temp_if_again->next;
	}
	free(head_Booked);

	printf("\n请输入姓名：");
	scanf_s("%s",name,20);



	//写入waitpeo.txt
	fopen_s(&fpWaitPeo,"WaitPeo.txt","a");
	fprintf_s(fpWaitPeo, "\n");//换行
	fprintf_s(fpWaitPeo, "%lld ", ID);//身份证号
	fprintf_s(fpWaitPeo, "%s ", name);//名字
	fprintf_s(fpWaitPeo, "%s ", now->Num_Train);//车编号
	fprintf_s(fpWaitPeo, "%s ", now->Station_Begin);//起点站
	fprintf_s(fpWaitPeo, "%s ", now->Station_End);//终点站
	fprintf_s(fpWaitPeo, "%ld ", now->date_Begin);//日期
	fprintf_s(fpWaitPeo, "%d ", now->time_Begin);//时
	fprintf_s(fpWaitPeo, "%d ", now->second_Begin);//分
	fprintf_s(fpWaitPeo, "%d ", level);//席别
	fprintf_s(fpWaitPeo, "%d", idroute);//ID-Route
	fclose(fpWaitPeo);

	printf("添加候补成功！\n");
	Line();

	return 1;
}

// 查询车次
void Search_Route() 
{
	//路线加载
	InitList_Route(fpRoute);

	struct Route* temp = head_Route;
	char search_End[10];
	char search_Begin[10];
	int count = 0;
	Line();
	char ch = getchar();
	printf("输入起点站：");
	gets_s(search_Begin, 10);
	printf("输入终点站：");
	gets_s(search_End,10);

	printf("\nID\t车次\t起点站\t到达站\t出发时间        到站时间\t特等座\t一等座\t二等座\t总票\t已购票\t剩余票\t待候补\n");

	while (temp->next != NULL)
	{

		if ( strcmp(temp->Station_End, search_End) == 0  && strcmp(temp->Station_Begin, search_Begin) == 0)
		{
			count++;
			printf("%d\t%s   %s  %s  %ld-%d:%d  %ld-%d:%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
					temp->id_Route,
					temp->Num_Train,
					temp->Station_Begin,
					temp->Station_End,
					temp->date_Begin,
					temp->time_Begin,
					temp->second_Begin,
					temp->date_End,
					temp->time_End,
					temp->second_End,
					temp->Level[0],
					temp->Level[1],
					temp->Level[2],
					temp->SumPeople,
					temp->Booked,
					temp->Remain,
					temp->Wait

					);

			//记录查询结果的地址 

		}
		temp = temp->next;
	}

	if (strcmp(temp->Station_End, search_End) == 0 && strcmp(temp->Station_Begin, search_Begin) == 0)
	{
		count++;
		printf("%d\t%s  %s  %s  %ld-%d:%d  %ld-%d:%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
			temp->id_Route,
			temp->Num_Train,
			temp->Station_Begin,
			temp->Station_End,
			temp->date_Begin,
			temp->time_Begin,
			temp->second_Begin,
			temp->date_End,
			temp->time_End,
			temp->second_End,
			temp->Level[0],
			temp->Level[1],
			temp->Level[2],
			temp->SumPeople,
			temp->Booked,
			temp->Remain,
			temp->Wait
		);

	}	
	printf("\n");
	if (count ==  0)
	{
		printf("\n^^^^^当前没有合适的车次^^^^^^\n\n");
	}
	else
	{
		printf("\n找到%d个车次\n",count);
	}
	Line();
}

//订车票
void Book_Ticket()
{
	InitList_Route(fpRoute);

	int flag = 0;
	struct Route* temp = head_Route;
	_Bool find  = FALSE;

start:
	printf("\n输入欲订车次的 ID ：");
	scanf_s("%d",&flag);
	
	//找该ID是否正确
	while (temp->next != NULL)
	{
		if (flag == temp->id_Route)
		{
			find = TRUE;
			printf("\nID\t车次\t起点站\t到达站\t出发时间        到站时间\t特等座\t一等座\t二等座\t总票\t已购票\t剩余票\t待候补\n");
			printf("%d\t%s   %s  %s  %ld-%d:%d  %ld-%d:%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
				temp->id_Route,
				temp->Num_Train,
				temp->Station_Begin,
				temp->Station_End,
				temp->date_Begin,
				temp->time_Begin,
				temp->second_Begin,
				temp->date_End,
				temp->time_End,
				temp->second_End,
				temp->Level[0],
				temp->Level[1],
				temp->Level[2],
				temp->SumPeople,
				temp->Booked,
				temp->Remain,
				temp->Wait
			);
			break;
		}		
		
		temp = temp->next;
	}
	if (flag == temp->id_Route && find ==  FALSE)
	{
		find = TRUE;
		printf("\nID\t车次\t起点站\t到达站\t出发时间        到站时间\t特等座\t一等座\t二等座\t总票\t已购票\t剩余票\t待候补\n");
		printf("%d\t%s   %s  %s  %ld-%d:%d  %ld-%d:%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
			temp->id_Route,
			temp->Num_Train,
			temp->Station_Begin,
			temp->Station_End,
			temp->date_Begin,
			temp->time_Begin,
			temp->second_Begin,
			temp->date_End,
			temp->time_End,
			temp->second_End,
			temp->Level[0],
			temp->Level[1],
			temp->Level[2],
			temp->SumPeople,
			temp->Booked,
			temp->Remain,
			temp->Wait
		);
	}

	if (find)//如果ID无误
	{
		Line();
		printf("\n确认信息车次无误请输入: 1\n其他返回主程序输入: 0  \n请输入：");
		int a;
		scanf_s("%d",&a);

start1:
		if (a == 1)
		{
			Line();
			//判断 座位是否足够
			int temp_Level;
			printf("\n特等座 一等座 二等座\n分别输入: 0 1 2\n");
			printf("输入选择:");
			scanf_s("%d",&temp_Level);
			if (temp_Level == 0)
			{
				if (temp->Level[0] <= 0)//特等座没票
				{
					Line();
					printf("特等座票数不足\n选择候补输入:1\n重新选择座席输入:2 \n请输入:");
					int select1;
					scanf_s("%d",&select1);
					if (select1 == 2)//重新选择
					{
						goto start1;
					}
					else if (select1 == 1)//选择候补
					{
						int aa = Select_wait(flag,temp_Level,temp);
						if (aa == 1) { temp->Wait++; }
						//更新 Route.text 文件  覆盖
						Update_FileRoute(head_Route);

						return;
					}
					else
					{ 
						goto start1;
					}

				}
				
			}
			else if (temp_Level == 1)
			{
				if (temp->Level[1] <= 0)//一等座没票
				{
					Line();
					printf("一等座票数不足\n选择候补输入:1\n重新选择座席输入:2 \n请输入:");
					int select1;
					scanf_s("%d", &select1);
					if (select1 == 2)//重新选择
					{
						goto start1;
					}
					else if (select1 == 1)//选择候补
					{
						int aa = Select_wait(flag, temp_Level, temp);
						if (aa == 1) { temp->Wait++; }
						//更新 Route.text 文件  覆盖
						Update_FileRoute(head_Route);

						return;
					}
					else
					{
						goto start1;
					}
				}
				
			}
			else if (temp_Level == 2)
			{
				if (temp->Level[2] <= 0)//二等座没票
				{
					Line();
					printf("二等座票数不足\n选择候补输入:1\n重新选择座席输入:2 \n请输入:");
					int select1;
					scanf_s("%d", &select1);
					if (select1 == 2)//重新选择
					{
						goto start1;
					}
					else if (select1 == 1)//选择候补
					{
						int aa = Select_wait(flag, temp_Level, temp);
						if (aa == 1) { temp->Wait++; }
						//更新 Route.text 文件  覆盖
						Update_FileRoute(head_Route);
						return;
					}
					else
					{
						goto start1;
					}
				}		
			}
			else // 输入错误
			{
				printf("你输入的指令有错，请退出\n");
				return;
			}


			//先判断是否已经买过该车次的车票
			InitList_BookedPeo(fpBookedPeo);
			struct user* temp_if_again = head_Booked;
			char tempname[20];
			long long int tempid;

			printf("输入姓名：");
			char ch = getchar();
			gets_s(tempname,20);
			printf("输入身份证号：");
			scanf_s("%lld", &tempid);//身份证号

			while (temp_if_again != NULL)
			{
				if (temp_if_again->Id == tempid)
				{
					Line();
					printf("每个证件号最多只能购买一张同一车次的车票！请勿重复购买");
					Line();
					return;
					free(head_Booked);
				}
				temp_if_again = temp_if_again->next;
			}

			free(head_Booked);

			// 购票人 头节点  信息输入
			head_Booked = (struct user*)malloc(sizeof(struct user));
			head_Booked->Level = temp_Level;//写入席别
			head_Booked->Id = tempid;//身份证号
			strcpy_s(head_Booked->Name, 20, tempname);//姓名
			head_Booked->ID_Route = flag;//路线ID
			strcpy_s(head_Booked->Station_Begin,10,temp->Station_Begin);//起点站	
			strcpy_s(head_Booked->Station_End,10, temp->Station_End);//终点站
			head_Booked->date_Begin = temp->date_Begin ;//日期
			head_Booked->time_Begin = temp->time_Begin;//时
			head_Booked->second_Begin = temp->second_Begin;//分
			strcpy_s(head_Booked->Num_Train, 7, temp->Num_Train);//起点站	




			//刷新routelist
			if (temp_Level == 0)
			{
				temp->Level[0]--;//特等座减一
				temp->Remain--;
				temp->Booked++;

			}
			else if (temp_Level == 1)
			{
				temp->Level[1]--;//特等座减一
				temp->Remain--;
				temp->Booked++;
			}
			else if (temp_Level == 2)
			{
				temp->Level[2]--;//特等座减一
				temp->Remain--;
				temp->Booked++;
			}


			
			//购票成功
			Line();
			printf("恭喜你 订票成功^_^\n");
			printf("车编号：%s\n发车时间：%ld-%d:%d\n", temp->Num_Train, temp->date_Begin, temp->time_Begin, temp->second_Begin);
			printf("请提前30mins到底检票处^_^\n");
			Line();

			//更新 Route.text 文件  覆盖
			Update_FileRoute(head_Route);

			//写入 BookedPeo.txt
			Write_Booked(head_Booked);
				
		}
		else if(a == 0)
		{
			/*Line();
			temp = head_Route;
			goto start;*/
			Line();
			return;
		}
		else
		{
			Line();
			return;
		}


	}
	else//ID不正确
	{
		printf("输入的ID有误，请检查！\n");
		goto start;
	}



}

//退订车票
void Cancel_Ticket()
{	
start4:
	long long int ID;
	int id_route;

	Line();
	printf("进入退票程序\n");
	printf("输入身份证号码：");
	scanf_s("%lld", &ID);
	printf("请输入路线ID：");
	scanf_s("%d", &id_route);

	InitList_BookedPeo(fpBookedPeo);//加载BookedList
	InitList_waitedPeo(fpWaitedPeo);//加载 候补成功List

	//遍历找ID
	struct user* temp_Booked = head_Booked;
	struct user* temp_Waited = head_Waited;

	//记录退订票的信息
	long long int cancel_id_route = id_route;
	int cancel_level;

	//标记是否找到
	_Bool find_Booked = FALSE;
	_Bool find_Waited = FALSE;

	//找是否已经购票 并且输出所有相关车票信息
	while (temp_Booked->next != NULL)
	{
		if (ID == temp_Booked->Id && id_route == temp_Booked->ID_Route)//找到
		{	
			find_Booked = TRUE;
			cancel_level = temp_Booked->Level;

			//检查是否为自动候补的用户
			while (temp_Waited->next != NULL)
			{
				if (ID == temp_Waited->Id && id_route == temp_Waited->ID_Route)//确认是 自动候补 的用户
				{
					find_Waited = TRUE;
					break;
				}
				temp_Waited = temp_Waited->next;
			}
		}
		temp_Booked = temp_Booked->next;
	}
	//最后一个节点之 前 仍未找到
	if (find_Booked == FALSE)
	{
		if (ID == temp_Booked->Id && id_route == temp_Booked->ID_Route)//找到
		{
			find_Booked = TRUE;
			cancel_level = temp_Booked->Level;

			//检查是否为自动候补的用户
			while (temp_Waited->next != NULL)
			{
				if (ID == temp_Waited->Id && id_route == temp_Waited->ID_Route)//确认是 自动候补 的用户
				{
					find_Waited = TRUE;
					break;
				}
			}
		}

	}


	// 找到购票记录 + 是自动候补
	if (find_Booked == TRUE && find_Waited == TRUE)
	{
		InitList_waitedPeo(fpWaitedPeo);//加载自动候补成功的用户
		Update_Waited(head_Waited, ID);//刷新WaitedList,更新WaitedPeo.txt

		InitList_BookedPeo(fpBookedPeo);//加载BookedpeoList
		Update_Booked(head_Booked, ID);//刷新BookedPeoList,更新Booked.txt

		//更新Route  遍历找到目标节点 更新数据后 更新文件
		_Bool find_route = FALSE;
		struct Route* temp_route = head_Route;
		if (temp_route == NULL)
		{
			return;
		}

		while (temp_route->next != NULL)//遍历找该节点并更新List
		{
			if (temp_Waited->ID_Route == temp_route->id_Route)//找到了目的线路
			{
				find_route = TRUE;
				//更新票数
				if (temp_Waited->Level == 0)//特等座
				{
					temp_route->Level[0]++;
				}
				else if (temp_Waited->Level == 1)//一等座
				{
					temp_route->Level[1]++;
				}
				else if (temp_Waited->Level == 2)//二等座
				{
					temp_route->Level[2]++;
				}
				temp_route->Remain++;//余票＋1
				temp_route->Booked--;//已购-1
				break;
			}
			temp_route = temp_route->next;
		}

		if (find_route == FALSE)
		{
			if (temp_Waited->ID_Route == temp_route->id_Route)//找到了目的线路
			{
				find_route = TRUE;
				//更新票数
				if (temp_Waited->Level == 0)//特等座
				{
					temp_route->Level[0]++;
				}
				else if (temp_Waited->Level == 1)//一等座
				{
					temp_route->Level[1]++;
				}
				else if (temp_Waited->Level == 2)//二等座
				{
					temp_route->Level[2]++;
				}

				temp_route->Remain++;//余票＋1
				temp_route->Booked--;//已购-1
			}
		}
		Update_FileRoute(head_Route);//更新route文件


		//检测待候补名单 找到符合条件的用户 转入自动补票
		InitList_WaitPeo();//加载待候补名单
		Find_WaitPeo(id_route, cancel_level);//遍历找是否有符合要求的

	}

	//找到购票记录  +  非自动候补
	if (find_Booked == TRUE && find_Waited == FALSE)
	{
		InitList_BookedPeo(fpBookedPeo);//加载BookedpeoList
		Update_Booked(head_Booked, ID);//刷新BookedPeoList , 更新Booked.txt


		//更新Route  遍历找到目标节点 更新数据后 更新文件
		InitList_Route(fpRoute);
		_Bool find_route = FALSE;
		struct Route* temp_route = head_Route;
		if (temp_route == NULL)
		{
			return;
		}
		//遍历找该节点并更新List
		while (temp_route->next != NULL)
		{
			if (temp_Waited->ID_Route == temp_route->id_Route)//找到了目的线路
			{
				find_route = TRUE;

				//更新票数
				if (temp_Waited->Level == 0)//特等座
				{
					temp_route->Level[0]++;
				}
				else if (temp_Waited->Level == 1)//一等座
				{
					temp_route->Level[1]++;
				}
				else if (temp_Waited->Level == 2)//二等座
				{
					temp_route->Level[2]++;
				}
				temp_route->Remain++;//余票＋1
				temp_route->Booked--;//已购-1

				break;
			}
			temp_route = temp_route->next;
		}
		if (find_route == FALSE)
		{
			if (temp_Waited->ID_Route == temp_route->id_Route)//找到了目的线路
			{
				find_route = TRUE;
				//更新票数
				if (temp_Waited->Level == 0)//特等座
				{
					temp_route->Level[0]++;
				}
				else if (temp_Waited->Level == 1)//一等座
				{
					temp_route->Level[1]++;
				}
				else if (temp_Waited->Level == 2)//二等座
				{
					temp_route->Level[2]++;
				}

				temp_route->Remain++;//余票＋1
				temp_route->Booked--;//已购-1
			}
		}

		//更新route文件
		Update_FileRoute(head_Route);
		Line();
		printf("您已经成功退票 ^_^  ^_^\n");
		Line();

		//检测待候补名单 找到符合条件的用户 转入自动补票
		Find_WaitPeo(id_route, cancel_level);//遍历找是否有符合要求的

	}

	//找不到购票记录
	if(find_Booked == FALSE && find_Waited == FALSE)
	{
			printf("该身份证号码未订票，请查询车票信息后重试！\n输入其他返回主菜单，重新输入身份证输入：1 \n请输入：");
			int select;
			scanf_s("%d",&select);
			if (select == 1)
			{
				goto start4;
			}
			else
			{
				return;
			}

	}
	

}

//个人查票
void Search_Inf()
{
	_Bool find = FALSE;
	long long int wait_ID;
start2:
	printf("你已经进入个人车票查询流程\n");
	printf("请输入你的身份证号码：");
	scanf_s("%lld", &wait_ID);
	Line();
	printf("查到如下信息：\n");

	//查找身份证号
	struct user* temp_1;
	//FreeList_User(head_Booked);
	InitList_BookedPeo(fpBookedPeo);

	temp_1 = head_Booked;

	char ch[7];
	char L0[] = "特等座";
	char L1[] = "一等座";
	char L2[] = "二等座";

	while (temp_1->next != NULL)
	{
		if (temp_1->Id == wait_ID)
		{
			find = TRUE;

			if (temp_1->Level == 0)
			{
				strcpy_s(ch, 7, L0);
			}
			else if (temp_1->Level == 1)
			{
				strcpy_s(ch, 7, L1);

			}
			else if (temp_1->Level == 2)
			{
				strcpy_s(ch, 7, L2);
			}

			//输出信息
			printf("\n路线ID: %d 姓名：%s 车次：%s 起点站：%s 终点站：%s 出发时间：%ld-%d:%d 席别：%s\n",
				temp_1->ID_Route,
				temp_1->Name,
				temp_1->Num_Train,
				temp_1->Station_Begin, 
				temp_1->Station_End,
				temp_1->date_Begin, 
				temp_1->time_Begin,
				temp_1->second_Begin,
				ch
			);

		}

		temp_1 = temp_1->next;
	
	}
	if (temp_1->Id == wait_ID)//判断最后一个数据
	{
		find = TRUE;
		if (temp_1->Level == 0)
		{
			strcpy_s(ch, 7, L0);
		}
		else if (temp_1->Level == 1)
		{
			strcpy_s(ch, 7, L1);

		}
		else if (temp_1->Level == 2)
		{
			strcpy_s(ch, 7, L2);
		}


		printf("\n路线ID: %d 姓名：%s 车次：%s 起点站：%s 终点站：%s 出发时间：%ld-%d:%d 席别：%s\n",
			temp_1->ID_Route,
			temp_1->Name,
			temp_1->Num_Train,
			temp_1->Station_Begin,
			temp_1->Station_End,
			temp_1->date_Begin,
			temp_1->time_Begin,
			temp_1->second_Begin,
			ch
		);

	}

	if (find == TRUE)
	{
		Line();
	}

	if (find == FALSE)
	{
		printf("\n查询不到相关记录\n");
		Line();
		return;
	}

}

//候补情况查询
void Search_wait()

{
	long long int ID;
	printf("你已经进入候补结果查询\n");
	printf("输入身份证号码查询：");
	scanf_s("%lld",&ID);

	//加载待候补 和 候补成功 名单
	InitList_waitedPeo();
	InitList_WaitPeo();


	//先检查 候补成功
	struct user* temp_waited = head_Waited;
	struct user* temp_wait = head_Wait;
	_Bool findWaited = FALSE;
	_Bool findWait = FALSE;


	//开始找候补成功的名单
	while(temp_waited->next != NULL)
	{
		if (ID ==temp_waited->Id)//找到
		{
			findWaited = TRUE;
			char ch[7];
			char L0[] = "特等座";
			char L1[] = "一等座";
			char L2[] = "二等座";
			if (temp_waited->Level == 0)
			{
				strcpy_s(ch, 7, L0);
			}
			else if (temp_waited->Level == 1)
			{
				strcpy_s(ch, 7, L1);

			}
			else if (temp_waited->Level == 2)
			{
				strcpy_s(ch, 7, L2);
			}

			Line();
			printf("\n^_^  ^_^已经候补成功！当前候补成功的车票信息为：\n");
			printf("\n路线ID：%d 姓名：%s 车编号：%s 席别：%s 起点站：%s 终点站：%s 出发时间：%ld-%d:%d\n",
				temp_waited->ID_Route,
				temp_waited->Name,
				temp_waited->Num_Train,
				ch,
				temp_waited->Station_Begin,
				temp_waited->Station_End,
				temp_waited->date_Begin,
				temp_waited->time_Begin,
				temp_waited->second_Begin			
				);
			Line();
		}
		temp_waited = temp_waited->next;
	}
	//没找到，进入最后一个节点
	if (findWaited == FALSE)
	{
		if (ID == temp_waited->Id)//找到
		{
			findWaited = TRUE;
			char ch[7];
			char L0[] = "特等座";
			char L1[] = "一等座";
			char L2[] = "二等座";
			if (temp_waited->Level == 0)
			{
				strcpy_s(ch, 7, L0);
			}
			else if (temp_waited->Level == 1)
			{
				strcpy_s(ch, 7, L1);

			}
			else if (temp_waited->Level == 2)
			{
				strcpy_s(ch, 7, L2);
			}

			Line();
			printf("\n^_^  ^_^已经候补成功！\n当前候补成功的车票信息为：\n");
			printf("\n路线ID：%d 姓名：%s 车编号：%s 席别：%s 起点站：%s 终点站：%s 出发时间：%ld-%d:%d\n",
				temp_waited->ID_Route,
				temp_waited->Name,
				temp_waited->Num_Train,
				ch,
				temp_waited->Station_Begin,
				temp_waited->Station_End,
				temp_waited->date_Begin,
				temp_waited->time_Begin,
				temp_waited->second_Begin
			);
			Line();
		}
	}

	//候补成功名单找不到 ，再检查待候补名单
	if (findWaited == FALSE)
	{
		//开始找待候补的名单
		while (temp_wait->next != NULL)
		{
			if (ID == temp_wait->Id)//找到
			{
				findWait = TRUE;
				char ch[7];
				char L0[] = "特等座";
				char L1[] = "一等座";
				char L2[] = "二等座";
				if (temp_wait->Level == 0)
				{
					strcpy_s(ch, 7, L0);
				}
				else if (temp_wait->Level == 1)
				{
					strcpy_s(ch, 7, L1);

				}
				else if (temp_wait->Level == 2)
				{
					strcpy_s(ch, 7, L2);
				}

				Line();
				printf("\n仍在等待候补状态！当前等待候补车票信息为：\n");
				printf("\n路线ID：%d 姓名：%s 车编号：%s 席别：%s 起点站：%s 终点站：%s 出发时间：%ld-%d:%d\n",
					temp_wait->ID_Route,
					temp_wait->Name,
					temp_wait->Num_Train,
					ch,
					temp_wait->Station_Begin,
					temp_wait->Station_End,
					temp_wait->date_Begin,
					temp_wait->time_Begin,
					temp_wait->second_Begin
				);
				Line();
				break;
			}
			temp_wait = temp_wait->next;
		}
		//没找到，进入最后一个节点
		if (findWait == FALSE)
		{
			if (ID == temp_wait->Id)//找到
			{
				findWait = TRUE;

				char ch[7];
				char L0[] = "特等座";
				char L1[] = "一等座";
				char L2[] = "二等座";
				if (temp_wait->Level == 0)
				{
					strcpy_s(ch, 7, L0);
				}
				else if (temp_wait->Level == 1)
				{
					strcpy_s(ch, 7, L1);

				}
				else if (temp_wait->Level == 2)
				{
					strcpy_s(ch, 7, L2);
				}

				Line();
				printf("\n路线ID：%d 姓名：%s 车编号：%s 席别：%s 起点站：%s 终点站：%s 出发时间：%ld-%d:%d\n",
					temp_wait->ID_Route,
					temp_wait->Name,
					temp_wait->Num_Train,
					ch,
					temp_wait->Station_Begin,
					temp_wait->Station_End,
					temp_wait->date_Begin,
					temp_wait->time_Begin,
					temp_wait->second_Begin
				);
				Line();
			}
		}
	}
	
	//都检索不到，说明没有参加过候补
	if(findWaited == FALSE && findWait == FALSE )
	{
		Line();
		printf("\n该身份证号码:%lld \n无候补相关的信息\n",ID);
		Line();
	}

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////