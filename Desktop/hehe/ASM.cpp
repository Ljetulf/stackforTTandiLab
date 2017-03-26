#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <fstream>
#include <cstdlib>
#include <ios>
#include <assert.h>
#include <stdlib.h>
#include <cstdio>


using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef map <string,int> MAP_type;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct S //��������� �������
{
    unsigned int number_str; //����� ������, � ������� ���������� �����
    int number_cmd; //����� �������, ���� �������� ��������
    char type_token; //��� ������,N-�����,L-��������� � �����,U-������������� �����,C-�������,R-�������,X-����������� �������
} Token_inter;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct A//��������� ������
{
    int number_cmd; //����� �������
    int val_arg; //����� ���������
    char type_arg; //��� ���������, N-�����, R-�������, L-�����, E-�������
} SA;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
class ASM_h
{
    enum flags {CMD=01,LABEL_USAGE=02,LABEL_DECLARATION=04,REG=010,NUM=020,DEFAULT_FLAG=05};
    enum l_flags {NO_LABEL=-2,L_ERROR=-1};
    MAP_type MAP_h; //map �������
    MAP_type MAP_r; //map ���������
    MAP_type MAP_l; //map �����, �������������� ��� � ���������� �����
    vector <int> Labels;//������� �����, �������������� ������� � ����� ������-������
    unsigned int labelnum; //���������� �����, ����� ������� �����
    vector <Token_inter> Token;//������ �������� �������
    vector <SA> Command;//������ ������
    bool success;
    public:
        ASM_h(void);
        ~ASM_h(void);
        string itoaa(int);
        int Get_token(string &,ifstream & f);
        bool Isnum(string &);
        void Translate(void);
        void Convert_labels(void);
        void Print_error(int&,int);
        bool Semantic(void);
        void Create_obj(void);
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void ASM_h::Convert_labels()
{
    int token_label;
    for (unsigned int i=0;i<Labels.size();i++)//���� �� ������ �����
    {
        token_label=Labels[i];//���������� � ����� ������
        if ((token_label==NO_LABEL)||(token_label==L_ERROR))//���� � ����� ����� ���� ������
            continue;//����������� ��������� ����������� �����
        for (unsigned int j=0;(j<Token.size())&&(j<(unsigned) token_label);j++)//����� �� �������
        {
            if (Token[j].type_token!='C')//���� �� �������, �� ��� �������
                Labels[i]--;//� ��� ���� �� �� ������ ��� ��������/�����
        };
    };
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void ASM_h::Print_error(int& flague,int str)
{
    printf("Error: ");
    switch (flague)
    {
        case L_ERROR:
            printf("this label declared more than one time");
            break;
        case DEFAULT_FLAG:
            printf("excepted function or label declaration");
            break;
        case LABEL_USAGE:
            printf("excepted label usage");
            break;
        case NO_LABEL:
            printf("this label is not declared");
            break;
        case (CMD|REG):
            printf("excepted register or void");
            break;
        case (REG|NUM):
            printf("excepted register or number");
            break;
        case REG:
            printf("excepted register");
            break;
        case NUM:
            printf("excepted number");
            break;
        default:
            printf("unknown flag of argument");
            break;
    };
    printf(", line %d\n",str);
    success=false;
    flague=DEFAULT_FLAG;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ASM_h::Semantic(void)
{
    int flag=DEFAULT_FLAG;
    success=true;//������� �� ������������� ������
    Convert_labels();
    SA Comds;
    for (unsigned int i=0;i<Token.size();i++)
    {
        switch (Token[i].type_token)
        {
            case 'C':
                if (flag&CMD)
                {
                    switch (Token[i].number_cmd)
                    {
                        case 2://��������� � �������� ��������� �������,���� �������(pop)
                            flag=REG;
                            if (success)//���� �� ���� ������
                            {
                                Comds.number_cmd=Token[i].number_cmd;//����� � ������ ������
                                Comds.val_arg=0;
                                Comds.type_arg='E';//�� ������ ���� �� ����� ����������
                                if ((i==Token.size()-1)||(Token[i+1].type_token!='R'))//���� ��������� �������� �� ���������� ��� ��-�� �������
                                {
                                    Command.push_back(Comds);
                                    flag=CMD;
                                };
                            };
                            break;
                        case 0: case 3: case 4: case 5: case 6: case 17: case 18: case 20: //���������� �������, �� ����������� ����������
                            flag=DEFAULT_FLAG;//����� ���� �������/�����
                            if (success)//���� �� ����� �� ���� ������
                            {
                                Comds.number_cmd=Token[i].number_cmd;//����� � ������ ������
                                Comds.val_arg=0;
                                Comds.type_arg='E';
                                Command.push_back(Comds);
                            };
                            break;
                        case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16://��������� � �������� ��������� �����
                            flag=LABEL_USAGE;
                            if (success)
                            {
                                Comds.number_cmd=Token[i].number_cmd;//����� � ������ ������
                            };
                            break;
                        case 1: //��������� � �������� ��������� ����� ��� �������
                            flag=NUM|REG;
                            if (success)
                            {
                                Comds.number_cmd=Token[i].number_cmd;
                            };
                            break;
                        case 7: case 8: case 19://��������� � �������� ��������� �������
                            flag=REG;
                            if (success)
                            {
                                Comds.number_cmd=Token[i].number_cmd;
                            };
                            break;
                        default:
                            printf("Error: unknown command number, line %d\n",Token[i].number_str);
                            success=false;
                            flag=DEFAULT_FLAG;
                            break;
                    };
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                };
                break;
            case 'N':
                if (flag&NUM)//���� ������� �����
                {
                    if (success)//���� �� ���� ������, ��� ���� ����, �� ��� ������ ������ � ������ ������
                    {
                        Comds.val_arg=Token[i].number_cmd;
                        Comds.type_arg='N';
                        Command.push_back(Comds);
                        flag=DEFAULT_FLAG;
                    }
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                };
                break;
            case 'L':
                if (flag&LABEL_DECLARATION)//���� ��� � �����
                {
                    if (Labels[Token[i].number_cmd]==L_ERROR)//���� ����� ���������������� ����� ��� 1 ���
                        Print_error(flag=L_ERROR,Token[i].number_str);
                    flag=DEFAULT_FLAG;
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                };
                break;
            case 'U':
                if (flag&LABEL_USAGE)
                {
                    if (success)
                    {
                        if ((Labels[Token[i].number_cmd]!=L_ERROR)&&(Labels[Token[i].number_cmd]!=NO_LABEL))
                        {
                            Comds.val_arg=Labels[Token[i].number_cmd];
                            Comds.type_arg='L';
                            Command.push_back(Comds);
                            flag=DEFAULT_FLAG;
                        }
                        else if (Labels[Token[i].number_cmd]==L_ERROR)
                            Print_error(flag=L_ERROR,Token[i].number_str);
                        else
                            Print_error(flag=NO_LABEL,Token[i].number_str);
                    };
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                }
                break;
            case 'R':
                if (flag&REG)
                {
                    if (success)
                    {
                        Comds.val_arg=Token[i].number_cmd;
                        Comds.type_arg='R';
                        Command.push_back(Comds);
                        flag=DEFAULT_FLAG;
                    }
                }
                else
                {
                    Print_error(flag,Token[i].number_str);
                }
                break;
            case 'X':
                printf("Error: unknown token, line %d\n",Token[i].number_str);
                Print_error(flag,Token[i].number_str);
                break;
            default:
                printf("Error: unknown compilation error, line %d\n",Token[i].number_str);
                assert(0);
                break;
        };
    };
    return success;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
string ASM_h::itoaa (int n)
{
    char *s = new char[17];
    string w;
    if (n < 0)
    {
        n =-n;
        w = "-";
    }
    int i=0;
    do {
        s[i++]=n%10+'0';
        n-=n%10;
    } while (n /= 10);
    for (int j=i-1;j>=0;j--)
    {
        w+=s[j];
    }
    delete[] s;
    return w;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
int ASM_h::Get_token(string & s,ifstream & f)
{
    static int numstr=0;
    if (f.eof())//���� �� ��������� ��� ������
        return EOF;
    else
    {
        while (f.peek()=='\n')//���� ���������� ������� �� ����� ������
        {
            f.get(); //������ ������
            numstr++;//����������� ������� �����
        }
        f >> s;//��������� �� ����� ������,��������� ���������� �������
        return numstr;
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ASM_h::Isnum(string & s) //������� ���������� ������ ����� � ���������� �������(���� ���)
{
    unsigned int it;
    if ((!isdigit(s[0]))&&(((s[0]!='+')&&(s[0]!='-'))||(s.size()==1))) //��������� ���������� �����
        return false;
    for (it=1;it < s.size();it++)
    {
        if (!isdigit(s[it]))
            return false;
    }
    return true;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void ASM_h::Translate(void)
{
    string s;//�������� ������
    Token_inter One_token;
    labelnum=0;
    int nstr;
    ifstream f;
    f.open("input.txt",ios::in);
    if (f.fail())//���� �� ������� ������� ����
    {
        printf("input file open failed,aborting");
        assert(0);
    }
    MAP_h["end"]=0;//�������������� map �������, ������ �������, ��������������� � ������������� ����� �� ���������
    MAP_h["push"]=1;//����� ����� ��� �������� � �������� � ����
    MAP_h["pop"]=2;//��������� �� ����� ����� � ������� ���, ���� �������� � �������
    MAP_h["add"]=3;//��������� ��� ������� ����� � ����� � �������� ��������� � ����
    MAP_h["diff"]=4;//������� �������� �������������� � ���������� ����� � �����
    MAP_h["mul"]=5;//�������� �� ������� ����� � ����� � �������� ��������� � ����
    MAP_h["div"]=6;//������� ������� �������������� � ���������� ����� � �����
    MAP_h["inc"]=7;//����������� �������� � �������� �� 1
    MAP_h["dec"]=8;//��������� �������� � �������� �� 1
    MAP_h["jmp"]=9;//������� �� �����
    MAP_h["je"]=10;//������� �� �����, ���� ZF=1
    MAP_h["jz"]=10;//ZF=1
    MAP_h["jne"]=11;//������� �� �����, ���� ZF=0
    MAP_h["ja"]=12;//������� �� �����, ���� CF=0 & ZF=0
    MAP_h["jnb"]=13;//CF=0
    MAP_h["jae"]=13;//CF=0
    MAP_h["jna"]=14;//CF=1 or ZF=1
    MAP_h["jbe"]=14;//CF=1 or ZF=1
    MAP_h["jb"]=15;//CF=1
    MAP_h["call"]=16;//����� ������������
    MAP_h["ret"]=17;//������� �� ������������
    MAP_h["in"]=18;//���� ����� � ������ � ����
    MAP_h["out"]=19;//������� ����� �� �������� �� �����
    MAP_h["cmp"]=20;//���������� ��� ����� � �����
    MAP_r["ax"]=1;//�������������� map ���������
    MAP_r["bx"]=2;
    MAP_r["cx"]=3;
    MAP_r["dx"]=4;
    while ((nstr=ASM_h::Get_token(s,f))!=EOF)
    {
        One_token.number_str=nstr;
        if (MAP_h.find(s)!=MAP_h.end())  //���� ��� �������
        {
            One_token.number_cmd=MAP_h[s];
            One_token.type_token='C';
            Token.push_back(One_token);
        }
        else if (Isnum(s))   //���� ��� �����
        {
            One_token.number_cmd=atoi(s.c_str());
            One_token.type_token='N';
            Token.push_back(One_token);
        }
        else if (MAP_r.find(s)!=MAP_r.end()) //���� ��� �������
        {
            One_token.number_cmd=MAP_r[s];
            One_token.type_token='R';
            Token.push_back(One_token);
        }
        else if (s[s.size()-1]==':') //���� ��� ���������� �����(1:)
        {
            s.pop_back(); //������� :
            if (MAP_l.find(s)==MAP_l.end()) //���� ����� ��� �� ���� ���������(���������� ���� ����� ������)
            {
                MAP_l[s]=labelnum;
                Labels.push_back(Token.size());//���������� ����� ������
                One_token.number_cmd=labelnum++;
            }
            else//���� ����� � ����� ������ ��� �����������
            {
                if  (Labels[MAP_l[s]]==NO_LABEL)//���� �� �� ����� ������ �������� �������� �� ���� �����
                    Labels[MAP_l[s]]=Token.size();
                else
                    Labels[MAP_l[s]]=L_ERROR;//��, ��������� 2 ����� � ���������� ������
                One_token.number_cmd=MAP_l[s];
            };
            One_token.type_token='L';
            Token.push_back(One_token);
        }
        else if (s[0]==':') //���� ��� ������������� �����(jmp :1)
        {
            s.erase(0,1); //������� :
            if (MAP_l.find(s)==MAP_l.end()) //���� ����� ��� �� ���� ���������(���������� ���� ����� ������)
            {
                MAP_l[s]=labelnum;
                Labels.push_back(NO_LABEL);//���� ������� �� ���� �� �����, -2 �������� ��������, ��� ���� ������� �� �� �����
                One_token.number_cmd=labelnum++;
            }
            else//���� ����� � ����� ������ ��� �����������
            {
                One_token.number_cmd=MAP_l[s];
            };
            One_token.type_token='U';
            Token.push_back(One_token);
        }
        else
        {
            One_token.number_cmd=0;
            One_token.type_token='X';
            Token.push_back(One_token);
        };
    }
    f.close();
    ofstream g("outASM1.txt"); //��� ��� ��� �� ������� �������� ���� outASM1.txt
    g.write("str",sizeof("str"));
    g.write("cmd",sizeof("cmd"));
    g.write("tkn\n",sizeof("tkn\n"));
    for (unsigned int i=0;i<Token.size();i++)
    {
        g.write(itoaa(Token[i].number_str).c_str(),sizeof(itoaa(Token[i].number_str)));
        g.write(itoaa(Token[i].number_cmd).c_str(),sizeof(itoaa(Token[i].number_cmd)));
        g.write(&(Token[i].type_token),sizeof(Token[i].type_token));
        g.write("\n",sizeof("\n"));
    }
    for (unsigned int i=0;i<Labels.size();i++)
    {
        g.write(itoaa(i).c_str(),sizeof(itoaa(i)));//����� to_string �� ��������,itoa ����,���� ����� ������������,������� ����������� ����������� >:(
        g.write(itoaa(Labels[i]).c_str(),sizeof(itoaa(Labels[i])));
        g.write("\n",sizeof("\n"));
    }
    g.close();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void ASM_h::Create_obj(void)
{
    FILE* h;
    h=fopen("Objectfile.txt","w");
    for (unsigned int i=0;i<Command.size();i++)
    {
        fprintf(h,"%d %d %c\n",Command[i].number_cmd,Command[i].val_arg,Command[i].type_arg);
    }
    fclose(h);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ASM_h::ASM_h(void)
{
    Translate();//��� �� ��������� ������ �� ����� � ������
    if(Semantic())//���������� � ���������� �����-�������������� �������, �������������� � ��������� �������
    {
        printf("Compilation finished\n");
        Create_obj();//������� ��������� ����
    }
    else
    {
        printf("Compilation failed\n");
    };
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ASM_h::~ASM_h(void)
{

};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    ASM_h();//������ ����,����� ���������� �� �������
    return 0;
};
