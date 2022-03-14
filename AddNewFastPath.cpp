/*
 * @Author: LetMeFly
 * @Date: 2022-03-13 14:20:40
 * @LastEditors: LetMeFly
 * @LastEditTime: 2022-03-13 20:03:52
 */
#include <sys/stat.h> 
#include <windows.h>
#include <Shlobj.h>
#include <iostream>
#include <fstream>
#include <io.h>  // access
using namespace std;

#define PATH_LENGTH 10240
#define TFPATH_LENGTH 1024
char path[PATH_LENGTH];
char TFpath[TFPATH_LENGTH];

/* 获取用户Path并保存到path中 */
void getPath(const char* name, char* array, DWORD dwSize) {
    HKEY hKey;
    int ret = RegOpenKey(HKEY_CURRENT_USER, "Environment", &hKey);  // 打开键
    RegQueryValueEx(hKey, name, NULL, NULL, (LPBYTE)array, &dwSize);  // 提取内容
}

/* path中是否存在“TFpath” */
bool ifTFpathInPath() {
    int l = strlen(path);
    const char TFpath[] = "TFpath";
    int lTF = strlen(TFpath);
    for (int i = 0; i + lTF - 1 < l; i++) {
        bool ok = true;
        for (int j = 0; j < lTF; j++) {
            if (path[i + j] != TFpath[j]) {
                ok = false;
                break;
            }
        }
        if (ok)
            return true;
    }
    return false;
}

bool ifIsDict(const string path) {
    struct _stat buf;
    int result;
    result = _stat(path.c_str(), &buf);
    if (_S_IFDIR & buf.st_mode) {
        return true;  // 文件夹
    }
    else if (_S_IFREG & buf.st_mode) {
        return false;  // 文件
    }
    return false;  // 都不是
}

bool ifPathExists(const string path, bool createIfNotExists = false) {
    if (!access(path.c_str(), 0) && ifIsDict(path))  // 已存在
        return true;
    if (createIfNotExists) {
        #ifdef FirstTryUsingMkdir
        // 不支持%userprofile%
        for (int i = 0; i < path.size(); i++) {
            if (path[i] == '/' || path[i] == '\\') {
                mkdir(path.substr(0, i).c_str());  // 已存在的部分会创建失败
            }
        }
        return !mkdir(path.c_str());
        #else
        return !system(("mkdir " + path).c_str());
        #endif
    }
    return false;  // 不存在 且 不创建
}

bool ifAInB(const string a, const string b) {
    if (a.size() > b.size())
        return false;
    for (int i = 0; i + a.size() - 1 < b.size(); i++) {
        bool in = true;
        for (int j = 0; j < a.size(); j++) {
            if (a[j] != b[i + j]) {
                in = false;
                break;
            }
        }
        if (in)
            return true;
    }
    return false;
}

/* 通过重启资源管理器的方式让Path生效 */
void refreshPath() {
    system("echo @echo off > %Temp%\\ReStartExplorer.bat");
    system("echo taskkill /IM explorer.exe /F >> %Temp%\\ReStartExplorer.bat");
    system("echo start explorer >> %Temp%\\ReStartExplorer.bat");
    system("echo exit >> %Temp%\\ReStartExplorer.bat");
    system("start %Temp%\\ReStartExplorer.bat");
}

/* 修改注册表 key 为 newValue */
void changeReg(string key, string newValue) {
    char regname[] = "Environment";
    HKEY hkResult;
    int ret = RegOpenKey(HKEY_CURRENT_USER, regname, &hkResult);
    ret = RegSetValueEx(hkResult, key.c_str(), 0, REG_EXPAND_SZ, (unsigned char *)newValue.c_str(), newValue.size());
    RegCloseKey(hkResult);
    refreshPath();
}

/* 将s添加到用户Path中 */
void joinPath(const string s) {
    getPath("Path", path, PATH_LENGTH);
    changeReg("Path", path + s + ";");
}

string getUserProfilePath() {
    system("echo %userprofile% > 63d2802aaef0cb6b0b8a48c68b1fa465.txt");  // 刚刚此文件的md5
    ifstream instr("63d2802aaef0cb6b0b8a48c68b1fa465.txt", ios::in);
    string s;
    instr >> s;
    instr.close();
    system("del 63d2802aaef0cb6b0b8a48c68b1fa465.txt");
    return s;
}

/* 查看是否是首次使用该程序 并添加%userprofile%\LetMeFly\TFpath进环境变量 */
void checkIfFirstTimeAndAdd() {
    getPath("TFpath", TFpath, TFPATH_LENGTH);
    bool alreadyExists = false;
    if (strlen(TFpath)) {  // 有TFpath
        getPath("Path", path, PATH_LENGTH);
        if (ifAInB(TFpath, path)) {  // TFpath的值在Path中
            if (ifPathExists(TFpath, false)) {  // TFpath是可访问的路径
                alreadyExists = true;
            }
            else {
                if (ifPathExists(TFpath, true)) {  // 创建成功
                    alreadyExists = true;
                }
                else
                    alreadyExists = false;
            }
        }
        else {
            if (ifPathExists(TFpath, false)) {  // 有TFpath但TFpath不在Path中
                joinPath(TFpath);
                if (ifPathExists(TFpath, true)) {
                    alreadyExists = true;
                }                
            }
        }
    }
    if (!alreadyExists) {
        string TFpath = getUserProfilePath() + "\\LetMeFly\\TFpath";
        changeReg("TFpath", TFpath);
        joinPath(TFpath);
        ifPathExists(TFpath, true);
    }
}

string chooseAPath() {
    TCHAR szBuffer[MAX_PATH] = {0};   
    BROWSEINFO bi;   
    ZeroMemory(&bi,sizeof(BROWSEINFO));   
    bi.hwndOwner = NULL;   
    bi.pszDisplayName = szBuffer;   
    bi.lpszTitle = "从下面选文件夹目录:";   
    bi.ulFlags = BIF_RETURNFSANCESTORS;   
    LPITEMIDLIST idl = SHBrowseForFolder(&bi);     
    SHGetPathFromIDList(idl,szBuffer);
    return string(szBuffer);
}

void add1Fast(bool ifCall=true) {
    string path = chooseAPath();
    if (!path.size())
        return;
    string name;
    printf("请为快捷命令起个名字(最好不要包含空格等特殊字符)：");
    cin >> name;
    getPath("TFpath", TFpath, TFPATH_LENGTH);
    ofstream ostr((string(TFpath) + "\\" + name + ".bat").c_str());
    ostr << "explorer " << path << endl;
    ostr.close();
    if (ifCall)
        system(name.c_str());
}

class Test {
public:
    void test_ifAInB() {
        string a, b;
        while (cin >> a >> b) {
            cout << ifAInB(a, b) << endl;
        }
        exit(0);
    }

    void test_ifPathExists_false() {
        // Del\aaa.cpp
        string s;
        while (cin >> s) {
            cout << ifPathExists(s, false) << endl;
        }
        exit(0);
    }

    void test_ifIsDict() {
        string s;
        while (cin >> s) {
            cout << ifIsDict(s) << endl;
        }
        exit(0);
    }

    void test_getUserProfilePath() {
        cout << getUserProfilePath() << endl;
        exit(0);
    }

    void test_add1Fast() {
        add1Fast();
        system("pause");
        exit(0);
    }
};

int main() {
    // Test test;
    // test.test_add1Fast();

    checkIfFirstTimeAndAdd();
    add1Fast();

    return 0;
}