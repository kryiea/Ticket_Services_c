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



////////////////////////////////////////////////////////////////////////////数据结构///////////////////////////////////////////

//单一车 存储结构
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

// 用户信息结构
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////初始化////////////////////////////////////////////////////

//判断是否找到数据文件 包括：路线信息 已订票名单 候补名单 ----- 找到返回1 失败返回0
void FindDatabase(FILE* fpRoute, FILE* fpBookedPeo, FILE* fpWaitPeo,FILE* fpWaitedPeo)
{
	//fopen_s 成功返回0 不成功返回其他数值
	int a = fopen_s(&fpBookedPeo, "BookedPeo.txt", "r");
	int b = fopen_s(&fpWaitPeo, "WaitPeo.txt", "r");
	int c = fopen_s(&fpRoute, "Route.txt", "r");
	int d = fopen_s(&fpWaitedPeo, "WaitedPeo.txt", "r");

	if (a == 0 && b == 0 && c == 0 && d== 0)
	{
		printf("数据文件已找到\n");
	
	}
	else//文件丢失
	{
		printf("数据文件未找到\n");
		exit(0);
	}

	fclose(fpRoute);
	fclose(fpBookedPeo);
	fclose(fpWaitPeo);
	fclose(fpWaitedPeo);

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
	fopen_s(&fpWaitPeo,"WaitedPeo.txt","r");

	if (!feof(fpWaitPeo))//非空文件
	{
		head_Wait = (struct user*)malloc(sizeof(struct user));
		struct user* temp_Wait = head_Wait;


		while (!feof(fpWaitPeo))
		{
			fscanf_s(fpBookedPeo, "%lld", &temp_Wait->Id);//读身份证
			fscanf_s(fpBookedPeo, "%s", temp_Wait->Name, 20);//读取名字
			fscanf_s(fpBookedPeo, "%s", temp_Wait->Station_Begin, 10);//读起点站
			fscanf_s(fpBookedPeo, "%s", temp_Wait->Station_End, 10);//读终点站
			fscanf_s(fpBookedPeo, "%ld", &temp_Wait->date_Begin);//读日期
			fscanf_s(fpBookedPeo, "%d", &temp_Wait->time_Begin);//读时
			fscanf_s(fpBookedPeo, "%d", &temp_Wait->second_Begin);//读分
			fscanf_s(fpBookedPeo, "%d", &temp_Wait->Level);//读席别
			fscanf_s(fpBookedPeo, "%d", &temp_Wait->ID_Route);//读路线ID

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
	fopen_s(&fpWaitedPeo, "WaitededPeo.txt", "r");

	if (!feof(fpWaitedPeo))//非空文件
	{
		head_Waited = (struct user*)malloc(sizeof(struct user));
		struct user* temp_Waited = head_Waited;


		while (!feof(fpWaitedPeo))
		{
			fscanf_s(fpBookedPeo, "%lld", &temp_Waited->Id);//读身份证
			fscanf_s(fpBookedPeo, "%s", temp_Waited->Name, 20);//读取名字
			fscanf_s(fpBookedPeo, "%s", temp_Waited->Station_Begin, 10);//读起点站
			fscanf_s(fpBookedPeo, "%s", temp_Waited->Station_End, 10);//读终点站
			fscanf_s(fpBookedPeo, "%ld", &temp_Waited->date_Begin);//读日期
			fscanf_s(fpBookedPeo, "%d", &temp_Waited->time_Begin);//读时
			fscanf_s(fpBookedPeo, "%d", &temp_Waited->second_Begin);//读分
			fscanf_s(fpBookedPeo, "%d", &temp_Waited->Level);//读席别
			fscanf_s(fpBookedPeo, "%d", &temp_Waited->ID_Route);//读路线ID

			struct user* Node = (struct user*)malloc(sizeof(struct user));
			Node->next = NULL;
			temp_Waited->next = Node;
			temp_Waited = Node;

		}
		temp_Waited = NULL;
		free(temp_Waited);
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



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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
	//先写入第一条数据 测试数据
	fprintf_s(fptemp,"44092120098255710 测试姓名 G1002 测起点 测终点 20211212 15 30 1 2");


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

//更新waited 的链表 和 文件
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
		temp_front->next == NULL;
		free(temp_Waited);//释放
	}

	//将新的 List 写入更新WaitedPeo.txt
	Update_FileUser(head_Waited, "WaitedPeo.txt");


}

//更新Booked 的链表 和 文件
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
		temp_front->next == NULL;
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

//找是 否有能候补的
void Find_WaitPeo(struct user* head_waitpeo, int Idroute,int level)
{
	struct user* temp_FindWait = head_waitpeo;
	_Bool findwait = FALSE;

	if (temp_FindWait == NULL)
	{
		return;
	}

	while (temp_FindWait->next != NULL)
	{
		if (Idroute == temp_FindWait->ID_Route && level == temp_FindWait->Level)//符合候补条件
		{
			findwait = TRUE;
			//自动候补


			break;
		}
		temp_FindWait = temp_FindWait->next;
	}
	if (findwait == FALSE)//最后一个节点前 仍未找到
	{
		if (Idroute == temp_FindWait->ID_Route && level == temp_FindWait->Level)//符合候补条件
		{
			findwait = TRUE;
			//自动候补
		}
	}

	//找不到合适的
	if (findwait == FALSE)
	{
		printf("无需要自动候补的成员\n");
		return;
	}


}



// 查询车次
void Search_Route() 
{

	struct Route* temp = head_Route;
	char search[10];
	int count = 0;
	Line();
	printf("输入终点站：");
	char ch = getchar();
	gets_s(search,10);
	
	
	printf("\nID\t车次\t起点站\t到达站\t出发时间        到站时间\t特等座\t一等座\t二等座\t总票\t已购票\t剩余票\t待候补\n");

	while (temp->next != NULL)
	{

		if ( strcmp(temp->Station_End, search) == 0 )
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

	if (strcmp(temp->Station_End, search) == 0)
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
		printf("\n确认信息请输入: 1  \n重新选择ID输入: 0  \n输入其他返回主程序  \n");
		int a;
		scanf_s("%d",&a);

start1:
		if (a == 1)
		{
			//判断 座位是否足够
			int temp_Level;
			printf("\n特等座 一等座 二等座 分别输入 0 1 2\n");
			printf("输入需要预订的 席别:");
			scanf_s("%d",&temp_Level);
			if (temp_Level == 0)
			{
				if (temp->Level[0] <= 0)//特等座没票
				{
					printf("特等座票数不足，请重新选择座席\n");
					goto start1;
				}
				
			}
			else if (temp_Level == 1)
			{
				if (temp->Level[1] <= 0)//一等座没票
				{
					printf("一等座票数不足，请重新选择座席\n");
					goto start1;
				}
				
			}
			else if (temp_Level == 2)
			{
				if (temp->Level[2] <= 0)//二等座没票
				{
					printf("二等座票数不足，请重新选择座席\n");
					goto start1;
				}		
			}
			else // 输入错误
			{
				printf("你输入的指令有错，请退出\n");
				exit(0);
			}

			// 购票人 头节点  信息输入
			head_Booked = (struct user*)malloc(sizeof(struct user));
			head_Booked->Level = temp_Level;//写入席别
			printf("输入姓名：");
			char ch = getchar();
			gets_s(head_Booked->Name,20); //姓名
			printf("输入身份证号：");
			scanf_s("%lld", &head_Booked->Id);//身份证号
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
			Line();
			temp = head_Route;
			goto start;
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
	Line();
	long long int ID;

	printf("输入身份证号码：");
	scanf_s("%lld", &ID);
	InitList_BookedPeo(fpBookedPeo);//加载BookedList

	//遍历找ID
	struct user* temp_Booked;
	struct user* temp_Waited;

	temp_Booked = head_Booked;
	temp_Waited = head_Waited;

	_Bool find_Booked = FALSE;
	_Bool find_Waited = FALSE;


	while (temp_Booked->next != NULL)
	{
		if (ID == temp_Booked->Id)//找到
		{
			find_Booked = TRUE;
			InitList_waitedPeo(fpWaitedPeo);//加载 候补成功List
			//检查是否为自动候补的用户
			while (temp_Waited->next != NULL)
			{
				if (ID == temp_Waited->Id)//确认是 自动候补 的用户
				{
					find_Waited = TRUE;

					
					InitList_waitedPeo(fpWaitedPeo);//加载自动候补成功的用户
					Update_Waited(head_Waited,ID);//刷新WaitedList,更新WaitedPeo.txt

					InitList_BookedPeo(fpBookedPeo);//加载BookedpeoList
					Update_Booked(head_Booked,ID);//刷新BookedPeoList,更新Booked.txt



					//更新Route  遍历找到目标节点 更新数据后 更新文件
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
							else if(temp_Waited->Level == 1)//一等座
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



					//检测待候补名单 找到符合条件的用户 转入自动补票

					InitList_WaitPeo();//加载待候补名单
					//遍历找是否有符合要求的
					Find_WaitPeo(head_Wait,temp_Waited->ID_Route,temp_Waited->Level);



					//自动补票流程：自动填充姓名 身份证号

					//补票完成
					//Waitpeo更新 WaitedPeo更新
					//输出 成功订票的相关信息



				}
			}
			break;
		}
		temp_Booked = temp_Booked->next;
	}

	if (find_Booked == FALSE)//最后一个节点之前仍未找到
	{
		if (ID == temp_Booked->Id)//找到
		{
			find_Booked = TRUE;



		}

		//不存在该订票人
		else
		{
			printf("该身份证号码未订票，请检查后重试！\n输入其他返回主菜单，输入 1 重新输入身份证：");
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




}

//个人查票
void Search_Inf()
{
	_Bool find = FALSE;
	long long int wait_ID;
start2:
	printf("请输入你的身份证号码：");
	scanf_s("%lld", &wait_ID);

	//查找身份证号
	struct user* temp_1;
	//FreeList_User(head_Booked);
	InitList_BookedPeo(fpBookedPeo);

	temp_1 = head_Booked;

	char ch[7];
	char L0[] = "特等座";
	char L1[] = "一等座";
	char L2[] = "二等座";
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

	while (temp_1->next != NULL)
	{
		if (temp_1->Id == wait_ID)
		{
			find = TRUE;
			

			//输出信息
			Line();
			printf("查到如下信息：\n");
			printf("\n姓名：%s 车次：%s 起点站：%s 终点站：%s 出发时间：%ld-%d:%d 席别：%s\n",
				temp_1->Name,
				temp_1->Num_Train,
				temp_1->Station_Begin, 
				temp_1->Station_End,
				temp_1->date_Begin, 
				temp_1->time_Begin,
				temp_1->second_Begin,
				ch
			);
			Line();


		}
		temp_1 = temp_1->next;
	
	}
	if (temp_1->Id == wait_ID)//判断最后一个数据
	{
		find = TRUE;
		Line();
		printf("查到如下信息：\n");
		printf("\n姓名：%s 车次：%s 起点站：%s 终点站：%s 出发时间：%ld-%d:%d 席别：%s\n",
			temp_1->Name,
			temp_1->Num_Train,
			temp_1->Station_Begin,
			temp_1->Station_End,
			temp_1->date_Begin,
			temp_1->time_Begin,
			temp_1->second_Begin,
			ch
		);
		Line();
	}
	if (find == FALSE)
	{
		printf("请检查输入的证件号是否正确，查询不到相关记录\n");
		printf("重新输入证件号请输入( 1 ):");
		int a = 0;
		scanf_s("%d",&a);
		Line();
		if (a == 1)
		{
			goto start2;
		}
		else
		{
			return;
		}
	}

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main()
{
	//判断文件是否存在
	FindDatabase(fpRoute, fpBookedPeo, fpWaitPeo, fpWaitedPeo);
	//路线加载进链表
	InitList_Route(fpRoute);	
	//加载已购票人名单
	InitList_BookedPeo(fpBookedPeo);



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
			break;
		case 5:
			Search_Inf();
			break;
		case 9:
			system("CLS");
			AdminMenu(); //9.后台管理
			break;
		case 0:   //0.退出程序
			printf("欢迎下次使用\n");
			fclose(fpRoute);
			fclose(fpBookedPeo);
			fclose(fpWaitPeo);
			exit(0);
			break;

		default:
			printf("请重新输入\n");
			Sleep(800);
			system("CLS");
			break;
		}
	}


	return 0;
}





