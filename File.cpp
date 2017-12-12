#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stack>

#define HBORDER "━"
#define VBORDER "┃"  //定义边框样式

int count = 0;  //count用来统计文件数
FILE *fp;  //fp用来向文件中写入搜索结果

char Lower(char ch)
{   //将大写字母转换为小写字母
    if (ch >= 'A' && ch <= 'Z') return ch + 32;
    else return ch;
}

char Upper(char ch)
{   //将小写字母转换为大写字母
    if (ch >= 'a' && ch<= 'z') return ch - 32;
    else return ch;
}

unsigned long Pow(int i, int n = 10)
{   //求n的乘方
    unsigned long s = 1;
    while (i--) s *= n;
    return s;
}

void SetColor(int num)
{   //设置控制台字体颜色
    HANDLE hConsoleOutput=GetStdHandle(STD_OUTPUT_HANDLE);
    if(num==0) SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_INTENSITY|FOREGROUND_BLUE|BACKGROUND_INTENSITY|0x0070);  //蓝字
    if(num==1) SetConsoleTextAttribute(hConsoleOutput, BACKGROUND_INTENSITY|0x0070);  //黑字
    if(num==2) SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_INTENSITY|FOREGROUND_RED|BACKGROUND_INTENSITY|0x0070);  //红字
    if(num==3) SetConsoleTextAttribute(hConsoleOutput, BACKGROUND_INTENSITY|BACKGROUND_RED|BACKGROUND_GREEN);  //黄色背景
    if(num==4) SetConsoleTextAttribute(hConsoleOutput, BACKGROUND_INTENSITY|BACKGROUND_GREEN);  //绿色背景
    if(num==5) SetConsoleTextAttribute(hConsoleOutput, BACKGROUND_INTENSITY|BACKGROUND_BLUE|BACKGROUND_RED);  //紫色背景
}

template<typename T>
void PrintAndSave(T word, char *pre = "%s", int color = 1, bool isSave = true)
{   //输出word并保存到文本文档中，pre控制格式，color控制颜色，isSave控制是否保存到文件
    SetColor(color);  //设置颜色
    if (isSave) fprintf(fp, pre, word);
    printf(pre, word);  //向控制台输出文字
    SetColor(1);  //还原黑字
}

int CmpFileName(char *patName, char *relName)
{   //比较含通配符的串与目标字符串是否匹配
    switch (*patName)
    {
        case '\0': return !*relName;
        case '*': return CmpFileName(patName + 1, relName) || *relName && CmpFileName(patName, relName + 1);
        case '?': return *relName && CmpFileName(patName + 1, relName + 1);
        default: return Lower(*patName) == Lower(*relName) && CmpFileName(patName + 1, relName + 1);
    }
}

void GetFileType(char *path)
{   //获取文件的类型并输出
    int len = strlen(path);
    bool isEnd = false;
    std::stack<char> s;
    PrintAndSave(" ");
    if (!strchr(path, '.')) PrintAndSave("      ");  //文件名中不存在'.'，表示此文件无扩展名
    else
        for (int i = len - 1; i > len - 5; i--)
        {   //程序从右到左获取文件扩展名，遇到'.'结束。考虑到美观的问题，扩展名最多显示4位
            if (path[i] != '.' && isEnd == false) s.push(Upper(path[i]));
            else
            {
                isEnd = true;  //标记已找到'.'
                s.push(' ');
            }
        }
        while (!s.empty())
        {   //将栈中的字符输出
            PrintAndSave(s.top(), "%c");
            s.pop();
        }
        PrintAndSave("文件  ");
}

void GetFilePath(char *allPath, char *shortPath)
{   //将一个含有文件名的路径中的文件名去掉
    int i = strlen(allPath) - 1;
    while (allPath[i] != '\\')	i--;  //找到'\'
    allPath[i] = '\0';  //将'\'替换为'\0'，编译器不再读取'\0'后面的文件名
    strcat(shortPath, allPath);
}

void GetFileSize(unsigned long size)
{   //将字节数带上单位
    unsigned long n = size, i = 0;
    char pre[]=" KMGTP";  //定义倍数前缀字母
    while(n /= 1024) i++;  //统计倍数
    n = size * 100 / Pow(i, 1024);
    PrintAndSave(n/100.0, " %4.3g");
    PrintAndSave(pre[i], "%cB  ");  //输出转换后的结果
}

void Search(char *path, char *name = "*.*", bool children = false)
{   //文件搜索算法（核心）
    HANDLE hFile;  //搜索句柄
    WIN32_FIND_DATA FileInfo;  //搜索返回的文件信息
    int errCode = 0;  //errCode会返回搜索信息
    char curPath[MAX_PATH], nextPath[MAX_PATH], curFileName[MAX_PATH];  //curPath为当前搜索路径，nextPath为其一子文件夹路径
    strcpy(curPath, path);
    strcat(curPath, "\\*.*");  //执行curPath=path+"\\*.*"
    hFile = FindFirstFile(curPath, &FileInfo);  //搜索本文件夹的第一个文件
    while (hFile != INVALID_HANDLE_VALUE && errCode != ERROR_NO_MORE_FILES)  //判断文件路径是否存在并且是否已搜索完成（防止重复搜索）
    {
        strcpy(curFileName, FileInfo.cFileName);
        if (curFileName[0] != '.')   //排除名称为.的文件夹（自身）、排除名称为..的文件夹（上级目录）
        {
            if (CmpFileName(name, curFileName))  //比较当前文件是否与搜索字符串匹配
            {
                PrintAndSave(count+1, "%4d", 0);  //设置字体颜色为蓝色
                PrintAndSave(curFileName, "┃%-24s  ");  //输出文件名
                PrintAndSave("类型", "%s", 5);  //设置字体背景色为黄色
                if ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) PrintAndSave("文件夹", "   %-21s");
                else
                {
                    GetFileType(curFileName);
                    PrintAndSave("大小", "%s", 3);  //设置字体背景色为黄色
                    GetFileSize((FileInfo.nFileSizeHigh * MAX_PATH) + FileInfo.nFileSizeLow);  //输出文件或文件夹的大小
                }
                PrintAndSave("位于", "%s", 4);  //设置字体背景色为绿色
                PrintAndSave(path, " %s\n");  //输出文件或文件夹的位置信息
                count++;
            }
            if ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && children)  //判断当前是否为文件夹
            {
                strcpy(nextPath, path);
                strcat(nextPath, "\\");
                strcat(nextPath, curFileName);  //执行nextPath=path+"\\"+FileInfo.cFileName，形成子文件夹路径
                Search(nextPath, name, children);  //递归搜索子文件夹
            }
        }
        FindNextFile(hFile, &FileInfo);  //查找下一个文件
        errCode = GetLastError();  //返回搜索信息
    }
    CloseHandle(hFile);  //结束查找
}

/*下面几个以Draw开头的函数都是为了绘制界面准备的，对整个程序的影响不大*/
void DrawLine(int n = 39, char *ch = HBORDER, bool noNextLine = false)
{   //绘制分割线
    while (n--) printf("%s", ch);
    if (!noNextLine) printf("\n");
}

void DrawFront(int n, bool noNextLine = false)
{   //绘制欢迎界面的前部份内容或整行内容
    while (n--)
    {
        DrawLine(10, " ", true);
        printf(VBORDER);
        if (!noNextLine) printf("%62s\n", VBORDER);
    }
}

void DrawWord(char *word, int align = 0, int number = 0)
{   //绘制文字，align=0表示居中，align=1表示左对齐
	int length = strlen(word) + (number == 0 ? 0 : 2);
    int i = (79 - length - 18) / 2;
    DrawFront(1, true);
    if (align == 1) i = 5;
    DrawLine(i, " ", true);
    if (number == 0) printf("%s", word);
    else printf("%2d%s", number, word);
    if (align == 1) i = 59 - length - 4;
    DrawLine(i, " ", true);
    printf("%s\n", VBORDER);
}

void DrawSplit()
{   //绘制分割线
    DrawLine(10, " ", true);
    printf("┣");
    DrawLine(30, HBORDER, true);
    printf("┫\n");
}

void DrawHead()
{   //绘制头部
    DrawLine(10, " ", true);
    printf("┏");
    DrawLine(30, HBORDER, true);
    printf("┓\n");
    DrawFront(1, true);
    DrawLine(20, " ", true);
    PrintAndSave("文件搜索（控制台版）", "%s", 3, false);
    DrawLine(20, " ", true);
    printf("%s\n", VBORDER);
}

void DrawEnd()
{   //绘制结尾
    DrawLine(10, " ", true);
    printf("┗");
    DrawLine(30, HBORDER, true);
    printf("┛\n");
}

void ShowWelcome(int second)
{   //显示欢迎界面
    if (second == 0) return;

    DrawLine(2, "\n");
    DrawHead();
    DrawSplit();
    DrawFront(1);
    DrawWord("作者：Crazy Urus");
    DrawFront(1);
    DrawSplit();
    DrawFront(1);
    DrawWord("程序说明：1.本程序是为《数据结构》课程实践开发，可", 1);
    DrawWord("            以实现对计算机中文件较快的搜索。", 1);
    DrawWord("          2.本程序支持两种通配符：*、?，其中*代替", 1);
    DrawWord("            多个字符，?代替1个字符。", 1);
    DrawWord("          3.本程序支持将搜索结果保存为文本文档。", 1);
    DrawFront(1);
    DrawWord("秒后将自动进入程序界面……", 1, second);
    DrawFront(1);
    DrawEnd();
    Sleep(1000);  //等待1秒
    system("cls");
    system("color F0");
    ShowWelcome(second - 1);
}

int main()
{
    int err = 0;  //记录搜索返回值
    char choose, path[MAX_PATH], name[MAX_PATH], children;  //存储输入的路径和文件名
    system("color F0");  //设置控制台颜色
    ShowWelcome(10);  //显示欢迎界面
    PrintAndSave("文件搜索（控制台版）", "%s\n", 3, false);
    DrawLine();
    fp = fopen("C:\\result.txt", "w");  //将result.txt文件清空
    fclose(fp);
    fp = fopen("C:\\result.txt", "a");  //重新打开fp，为保存结果作准备
    do
    {
        if (err) PrintAndSave(path, "路径 %s 不存在！\n\n", 2, false);  //err=1代表路径出错
        printf("请输入搜索的位置：");
        scanf("%s", path);
    } while (err = access(path, 0));  //判断输入的路径是否存在
    printf("请输入文件名称(包含扩展名，跳过请填*):");
    scanf("%s", name);
    printf("是否在子文件夹中递归查找？ (Y/N) ");
    getchar();  //将换行符跳过
    scanf("%c", &children);
    DrawLine();
    fprintf(fp, "这是在 %s 中搜索 %s 的结果：\n\n", path, name);
    printf("正在为您查找，请稍候……\n");
    if (children == 'Y' || children == 'y') Search(path, name, true);  //搜索文件
    else Search(path, name, false);  //搜索文件
    DrawLine(2, HBORDER, true);
    printf("┻");
    DrawLine(36);
    fclose(fp);
    if (count)
    {
        printf("搜索完毕！共找到 %d 个符合要求的文件。\n", count);  //count大于0表示找到了文件
        printf("是否查看已保存到 C:\\result.txt 的搜索报告？ (Y/N)  ");
        getchar();  //将换行符跳过
        scanf("%c", &choose);
        if (choose == 'Y' || choose == 'y') system("notepad C:\\result.txt");
    }
    else printf("未找到符合要求的文件！\n");  //count=0表示未找到文件
    DrawLine();
    printf("感谢使用本程序！\n");
    return 0;
}