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

//购票人 存储结构
struct BookedPeo
{
	char Name[20];//姓名
	long long int Id;//身份证

	int ID_Train;//车编号 用于辨别车次
	char Station_Begin[10];//起点站
	char Station_End[10];//到达站

	long int date_Begin;//出发时间
	int time_Begin;//出发时
	int second_Begin;//出发分

	int Level;//席别

	struct BookedPeo* next;
};

//候补人 存储结构
struct WaitPeo
{
	char Name[20];//姓名
	long long int Id;//身份证
	int ID_Train;//目标车编号

	char Station_Begin[10];//起点站
	char Station_End[10];//到达站

	long int date_Begin;//出发时间
	int time_Begin;//出发时
	int second_Begin;//出发分

	int Level;//席别


	struct WaitPeo* next;

};



struct Route* head_Route;//路线头节点
struct BookedPeo* head_Booked;//已购票人头节点


//文件指针
FILE* fpRoute;
FILE* fpBookedPeo;
FILE* fpWaitPeo;
FILE* fptemp;



//判断是否找到数据文件 包括：路线信息 已订票名单 候补名单 ----- 找到返回1 失败返回0
void FindDatabase(FILE** fpRoute, FILE** fpBookedPeo, FILE** fpWaitPeo)
{
	//fopen_s 成功返回0 不成功返回其他数值
	int a = fopen_s(fpBookedPeo, "BookedPeo.txt", "r");
	int b = fopen_s(fpWaitPeo, "WaitPeo.txt", "r");
	int c = fopen_s(fpRoute, "Route.txt", "r");

	if (a == 0 && b == 0 && c == 0)
	{
		printf("数据文件已找到\n");
	
	}
	else//文件丢失
	{
		printf("数据文件未找到\n");
		exit(0);
	}
}

//读取 路线信息文件 载入链表
void InitList_Route(FILE** fpRoute_r, FILE* fpRoute)
{
	//建立头节点
	head_Route = (struct Route*)malloc(sizeof(struct Route));
	head_Route->next = NULL;

	//建立临时节点 用于刷新
	struct Route* temp;
	temp = head_Route;

	char ch;

	fopen_s(fpRoute_r, "Route.txt", "r");// fpRoute 指向Route.txt的文件流

	while (!feof(fpRoute))
	{
		fscanf_s(fpRoute,"%d",&temp->id_Route);//读入车id
		ch = fgetc(fpRoute);//吞掉 \n

		fgets(temp->Num_Train, 7, fpRoute);//读入车编号
		temp->Num_Train[5] = '\0';

		fgets(temp->Station_Begin, 8, fpRoute);//读入起点站	
		temp->Station_Begin[6] = '\0';

		fgets(temp->Station_End, 8, fpRoute);//读入终点站
		temp->Station_End[6] = '\0';

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
	fclose(fpRoute);

}

//读取 已定票名单文件 载入链表
void InitList_BookedPeo()
{
	fclose(fpBookedPeo);
	fopen_s(&fpBookedPeo, "BookedPeo.txt", "r");
	//载入
	head_Booked = (struct BookedPeo*)malloc(sizeof(struct BookedPeo));
	head_Booked->next = NULL;
	struct BookedPeo* temp_Booked = NULL;
	temp_Booked = head_Booked;
	while (!feof(fpBookedPeo))
	{
		fscanf_s(fpBookedPeo, "%lld", &temp_Booked->Id);
		//char ch = fgetc(fpBookedPeo);
		//fscanf_s(fpBookedPeo, "%s",temp_Booked->Name);
		//fgets(temp_Booked->Name,20,fpBookedPeo);
		//strcpy_s(temp_Booked->Name,strlen(temp),temp);
		//fscanf_s(fpBookedPeo, "%d", &temp_Booked->Level);
		//fscanf_s(fpBookedPeo, "%d", &temp_Booked->ID_Train);


		struct BookedPeo* Node = (struct BookedPeo*)malloc(sizeof(struct BookedPeo));
		Node->next = NULL;
		temp_Booked->next = Node;
		temp_Booked = Node;

	}

	//关闭文件
	fclose(fpBookedPeo);

}

//读取 候补名单文件 载入链表
void InitList_WaitPeo()
{

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

			//创建 购票人 头节点
			head_Booked = (struct BookedPeo*)malloc(sizeof(struct BookedPeo));
			head_Booked->Level = temp_Level;//写入席别
			printf("输入姓名：");
			char ch = getchar();
			gets_s(head_Booked->Name,20);
			printf("输入身份证号：");
			scanf_s("%lld", &head_Booked->Id);
			head_Booked->ID_Train = flag;

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


			//写入 BookedPeo.txt
			fclose(fpBookedPeo);
			fopen_s(&fpBookedPeo,"BookedPeo.txt","a");//指向文件尾 可写
			fprintf_s(fpBookedPeo,"%lld",head_Booked->Id);//购票人身份证号码
			fprintf_s(fpBookedPeo, " ");
			fputs(head_Booked->Name, fpBookedPeo);//购票人姓名
			fprintf_s(fpBookedPeo, " ");
			fprintf_s(fpBookedPeo, "%d", head_Booked->Level);//席别
			fprintf_s(fpBookedPeo, " ");
			fprintf_s(fpBookedPeo, "%d", head_Booked->ID_Train);//车id
			fprintf_s(fpBookedPeo, "\n");

			//购票成功
			Line();
			printf("恭喜你 订票成功^_^\n");
			printf("车编号：%s\n发车时间：%ld-%d:%d\n", temp->Num_Train, temp->date_Begin, temp->time_Begin, temp->second_Begin);
			printf("请提前30mins到底检票处^_^\n");
			Line();


			//更新 Route.text 文件  覆盖
			temp = head_Route;
			fopen_s(&fptemp,"temp.txt","w"); // 创建新文件
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
			
			//关闭
			fclose(fptemp);
			fclose(fpBookedPeo);
			free(head_Booked);//释放

			remove("Route.txt");
			rename("temp.txt","Route.txt");
				
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
	//输入 身份证 查 ，选 车ID
	//退票后，更新Roulist(车票数量的更新),更新txt
	//在已购人链表 删除ID那个节点 ，更新txt
	//进入判断 候补名单是否符合要求(加载进候补人链表) 遍历每一位候补人 通过检索ID，对比Level的数量是否>0,如果是自动进行订票，
	// 并更新候补人链表（删去那个节点）以及BookedPeo链表，更新 输出候补成功的信息，写入候补成功名单txt 和 成功购票人txt（重新覆盖）
	// 
	//


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
	struct BookedPeo* temp_1;
	temp_1 = head_Booked;
	while (temp_1->next->next != NULL)
	{
		if (temp_1->Id == wait_ID)
		{
			find = TRUE;
			//输出信息

		}
		temp_1 = temp_1->next;
	
	}
	if (temp_1->Id == wait_ID)//判断最后一个数据
	{
		find = TRUE;
		//输出信息

	}
	if (find == FALSE)
	{
		printf("请检查输入的证件号是否正确，查询不到相关记录\n");
		printf("重新输入证件号请输入 1");
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


int main()
{
	//判断文件是否存在
	FindDatabase(&fpRoute, &fpBookedPeo, &fpWaitPeo);
	//路线加载进链表
	InitList_Route(&fpRoute,fpRoute);	
	//加载已购票人名单
	InitList_BookedPeo();



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
			Search_Route();//1.查询车票
			break;
		case 2:   
			//2.预订车票
			Book_Ticket();
			break;
		case 3:   
			//3.退订车票
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





