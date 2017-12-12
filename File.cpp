#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stack>

#define HBORDER "��"
#define VBORDER "��"  //����߿���ʽ

int count = 0;  //count����ͳ���ļ���
FILE *fp;  //fp�������ļ���д���������

char Lower(char ch)
{   //����д��ĸת��ΪСд��ĸ
    if (ch >= 'A' && ch <= 'Z') return ch + 32;
    else return ch;
}

char Upper(char ch)
{   //��Сд��ĸת��Ϊ��д��ĸ
    if (ch >= 'a' && ch<= 'z') return ch - 32;
    else return ch;
}

unsigned long Pow(int i, int n = 10)
{   //��n�ĳ˷�
    unsigned long s = 1;
    while (i--) s *= n;
    return s;
}

void SetColor(int num)
{   //���ÿ���̨������ɫ
    HANDLE hConsoleOutput=GetStdHandle(STD_OUTPUT_HANDLE);
    if(num==0) SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_INTENSITY|FOREGROUND_BLUE|BACKGROUND_INTENSITY|0x0070);  //����
    if(num==1) SetConsoleTextAttribute(hConsoleOutput, BACKGROUND_INTENSITY|0x0070);  //����
    if(num==2) SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_INTENSITY|FOREGROUND_RED|BACKGROUND_INTENSITY|0x0070);  //����
    if(num==3) SetConsoleTextAttribute(hConsoleOutput, BACKGROUND_INTENSITY|BACKGROUND_RED|BACKGROUND_GREEN);  //��ɫ����
    if(num==4) SetConsoleTextAttribute(hConsoleOutput, BACKGROUND_INTENSITY|BACKGROUND_GREEN);  //��ɫ����
    if(num==5) SetConsoleTextAttribute(hConsoleOutput, BACKGROUND_INTENSITY|BACKGROUND_BLUE|BACKGROUND_RED);  //��ɫ����
}

template<typename T>
void PrintAndSave(T word, char *pre = "%s", int color = 1, bool isSave = true)
{   //���word�����浽�ı��ĵ��У�pre���Ƹ�ʽ��color������ɫ��isSave�����Ƿ񱣴浽�ļ�
    SetColor(color);  //������ɫ
    if (isSave) fprintf(fp, pre, word);
    printf(pre, word);  //�����̨�������
    SetColor(1);  //��ԭ����
}

int CmpFileName(char *patName, char *relName)
{   //�ȽϺ�ͨ����Ĵ���Ŀ���ַ����Ƿ�ƥ��
    switch (*patName)
    {
        case '\0': return !*relName;
        case '*': return CmpFileName(patName + 1, relName) || *relName && CmpFileName(patName, relName + 1);
        case '?': return *relName && CmpFileName(patName + 1, relName + 1);
        default: return Lower(*patName) == Lower(*relName) && CmpFileName(patName + 1, relName + 1);
    }
}

void GetFileType(char *path)
{   //��ȡ�ļ������Ͳ����
    int len = strlen(path);
    bool isEnd = false;
    std::stack<char> s;
    PrintAndSave(" ");
    if (!strchr(path, '.')) PrintAndSave("      ");  //�ļ����в�����'.'����ʾ���ļ�����չ��
    else
        for (int i = len - 1; i > len - 5; i--)
        {   //������ҵ����ȡ�ļ���չ��������'.'���������ǵ����۵����⣬��չ�������ʾ4λ
            if (path[i] != '.' && isEnd == false) s.push(Upper(path[i]));
            else
            {
                isEnd = true;  //������ҵ�'.'
                s.push(' ');
            }
        }
        while (!s.empty())
        {   //��ջ�е��ַ����
            PrintAndSave(s.top(), "%c");
            s.pop();
        }
        PrintAndSave("�ļ�  ");
}

void GetFilePath(char *allPath, char *shortPath)
{   //��һ�������ļ�����·���е��ļ���ȥ��
    int i = strlen(allPath) - 1;
    while (allPath[i] != '\\')	i--;  //�ҵ�'\'
    allPath[i] = '\0';  //��'\'�滻Ϊ'\0'�����������ٶ�ȡ'\0'������ļ���
    strcat(shortPath, allPath);
}

void GetFileSize(unsigned long size)
{   //���ֽ������ϵ�λ
    unsigned long n = size, i = 0;
    char pre[]=" KMGTP";  //���屶��ǰ׺��ĸ
    while(n /= 1024) i++;  //ͳ�Ʊ���
    n = size * 100 / Pow(i, 1024);
    PrintAndSave(n/100.0, " %4.3g");
    PrintAndSave(pre[i], "%cB  ");  //���ת����Ľ��
}

void Search(char *path, char *name = "*.*")
{   //�ļ������㷨�����ģ�
    HANDLE hFile;  //�������
    WIN32_FIND_DATA FileInfo;  //�������ص��ļ���Ϣ
    int errCode = 0;  //errCode�᷵��������Ϣ
    char curPath[MAX_PATH], nextPath[MAX_PATH], curFileName[MAX_PATH];  //curPathΪ��ǰ����·����nextPathΪ��һ���ļ���·��
    strcpy(curPath, path);
    strcat(curPath, "\\*.*");  //ִ��curPath=path+"\\*.*"
    hFile = FindFirstFile(curPath, &FileInfo);  //�������ļ��еĵ�һ���ļ�
    while (hFile != INVALID_HANDLE_VALUE && errCode != ERROR_NO_MORE_FILES)  //�ж��ļ�·���Ƿ���ڲ����Ƿ���������ɣ���ֹ�ظ�������
    {
        strcpy(curFileName, FileInfo.cFileName);
        if (curFileName[0] != '.')   //�ų�����Ϊ.���ļ��У��������ų�����Ϊ..���ļ��У��ϼ�Ŀ¼��
        {
            if (CmpFileName(name, curFileName))  //�Ƚϵ�ǰ�ļ��Ƿ��������ַ���ƥ��
            {
                PrintAndSave(count+1, "%4d", 0);  //����������ɫΪ��ɫ
                PrintAndSave(curFileName, "��%-24s  ");  //����ļ���
                PrintAndSave("����", "%s", 5);  //�������屳��ɫΪ��ɫ
                if ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) PrintAndSave("�ļ���", "   %-21s");
                else
                {
                    GetFileType(curFileName);
                    PrintAndSave("��С", "%s", 3);  //�������屳��ɫΪ��ɫ
                    GetFileSize((FileInfo.nFileSizeHigh * MAX_PATH) + FileInfo.nFileSizeLow);  //����ļ����ļ��еĴ�С
                }
                PrintAndSave("λ��", "%s", 4);  //�������屳��ɫΪ��ɫ
                PrintAndSave(path, " %s\n");  //����ļ����ļ��е�λ����Ϣ
                count++;
            }
            if ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))  //�жϵ�ǰ�Ƿ�Ϊ�ļ���
            {
                strcpy(nextPath, path);
                strcat(nextPath, "\\");
                strcat(nextPath, curFileName);  //ִ��nextPath=path+"\\"+FileInfo.cFileName���γ����ļ���·��
                Search(nextPath, name);  //�ݹ��������ļ���
            }
        }
        FindNextFile(hFile, &FileInfo);  //������һ���ļ�
        errCode = GetLastError();  //����������Ϣ
    }
    CloseHandle(hFile);  //��������
}

/*���漸����Draw��ͷ�ĺ�������Ϊ�˻��ƽ���׼���ģ������������Ӱ�첻��*/
void DrawLine(int n = 39, char *ch = HBORDER, bool noNextLine = false)
{   //���Ʒָ���
    while (n--) printf("%s", ch);
    if (!noNextLine) printf("\n");
}

void DrawFront(int n, bool noNextLine = false)
{   //���ƻ�ӭ�����ǰ�������ݻ���������
    while (n--)
    {
        DrawLine(10, " ", true);
        printf(VBORDER);
        if (!noNextLine) printf("%62s\n", VBORDER);
    }
}

void DrawWord(char *word, int align = 0)
{   //�������֣�align=0��ʾ���У�align=1��ʾ�����
    int i = (79 - strlen(word) - 18) / 2;
    DrawFront(1, true);
    if (align == 1) i = 5;
    DrawLine(i, " ", true);
    printf("%s", word);
    if (align == 1) i = 59 - strlen(word) - 4;
    DrawLine(i, " ", true);
    printf("%s\n", VBORDER);
}

void DrawSplit()
{   //���Ʒָ���
    DrawLine(10, " ", true);
    printf("��");
    DrawLine(30, HBORDER, true);
    printf("��\n");
}

void DrawHead()
{   //����ͷ��
    DrawLine(10, " ", true);
    printf("��");
    DrawLine(30, HBORDER, true);
    printf("��\n");
    DrawFront(1, true);
    DrawLine(20, " ", true);
    PrintAndSave("�ļ�����������̨�棩", "%s", 3, false);
    DrawLine(20, " ", true);
    printf("%s\n", VBORDER);
}

void DrawEnd()
{   //���ƽ�β
    DrawLine(10, " ", true);
    printf("��");
    DrawLine(30, HBORDER, true);
    printf("��\n");
}

void ShowWelcome()
{   //��ʾ��ӭ����
    DrawLine(2, "\n");
    DrawHead();
    DrawSplit();
    DrawFront(1);
    DrawWord("���ߣ�����");
    DrawWord("�༶���������ZY1201");
    DrawFront(1);
    DrawSplit();
    DrawFront(1);
    DrawWord("����˵����1.��������Ϊ�����ݽṹ���γ�ʵ����������", 1);
    DrawWord("            ��ʵ�ֶԼ�������ļ��Ͽ��������", 1);
    DrawWord("          2.������֧������ͨ�����*��?������*����", 1);
    DrawWord("            ����ַ���?����1���ַ���", 1);
    DrawWord("          3.������֧�ֽ������������Ϊ�ı��ĵ���", 1);
    DrawFront(1);
    DrawWord("10����Զ����������桭��", 1);
    DrawFront(1);
    DrawEnd();
    Sleep(10000);  //�ȴ�10��
    system("cls");
    system("color F0");
}

int main()
{
    int err = 0;  //��¼��������ֵ
    char choose, path[MAX_PATH], name[MAX_PATH];  //�洢�����·�����ļ���
    system("color F0");  //���ÿ���̨��ɫ
    ShowWelcome();  //��ʾ��ӭ����
    PrintAndSave("�ļ�����������̨�棩", "%s\n", 3, false);
    DrawLine();
    fp = fopen("C:\\result.txt", "w");  //��result.txt�ļ����
    fclose(fp);
    fp = fopen("C:\\result.txt", "a");  //���´�fp��Ϊ��������׼��
    do
    {
        if (err) PrintAndSave(path, "·�� %s �����ڣ�\n\n", 2, false);  //err=1����·������
        printf("������������λ�ã�");
        scanf("%s", path);
    } while (err = access(path, 0));  //�ж������·���Ƿ����
    printf("�������ļ�����(������չ������������*):");
    scanf("%s", name);
    DrawLine();
    fprintf(fp, "������ %s ������ %s �Ľ����\n\n", path, name);
    printf("����Ϊ�����ң����Ժ򡭡�\n");
    Search(path, name);  //�����ļ�
    DrawLine(2, HBORDER, true);
    printf("��");
    DrawLine(36);
    fclose(fp);
    if (count)
    {
        printf("������ϣ����ҵ� %d ������Ҫ����ļ���\n", count);  //count����0��ʾ�ҵ����ļ�
        printf("�Ƿ�鿴�ѱ��浽 C:\\result.txt ���������棿 (Y/N)  ");
        getchar();  //�����з�����
        scanf("%c", &choose);
        if(choose == 'Y' || choose == 'y') system("notepad C:\\result.txt");
    }
    else printf("δ�ҵ�����Ҫ����ļ���\n");  //count=0��ʾδ�ҵ��ļ�
    DrawLine();
    printf("��лʹ�ñ�����\n");
    return 0;
}