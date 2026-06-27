
#include "common_mqh_cpp_PCH.h"

class SYSTEM_LOG_MANAGER : public LOG_MANAGER_BASE {

private:

    struct TIME_STR_P_DB {

        string Root_Y, Root_M, Root_D;
    };

    TIME_STR_P_DB Time_Db;

    virtual ulong Get_Root(const string& file_name, _vector<string>& result) override {

        std::tm Current_Time;

        WORD Milli_Sec;

        ulong Error_Code = Get_Current_Time(Current_Time, Milli_Sec);

        if (Error_Code != ERROR_SUCCESS)
            return Error_Code;

        result.push_back(MAKER_FOLDER_DRC());
        result.push_back(ITEM_FOLDER_DRC());
        result.push_back(ITEM_LOG_FOLDER_DRC());
        result.push_back(ITEM_SYSTEM_LOG_FOLDER_DRC());
        result.push_back(ITEM_SYSTEM_DAY_LOG_FOLDER_DRC());

        swprintf_s(Time_Db.Root_Y, 1024, L"\\%04d", Current_Time.tm_year + 1900);

        result.push_back(Time_Db.Root_Y);

        swprintf_s(Time_Db.Root_M, 1024, L"\\%02d", Current_Time.tm_mon + 1);

        result.push_back(Time_Db.Root_M);

        swprintf_s(Time_Db.Root_D, 1024, L"\\%02d", Current_Time.tm_mday);

        result.push_back(Time_Db.Root_D);

        return (uint)result.size();
    }

    virtual ulong Format_Log_Line(const ulong& error, const uint& line, const bool& error_type) override {

        std::tm Current_Time;

        WORD Milli_Sec;

        ulong Error_Code = Get_Current_Time(Current_Time, Milli_Sec);

        if (Error_Code != ERROR_SUCCESS)
            return Error_Code;

        _vector<string> Command_Line_Value;

        PROCESS_MANAGER Process_Mng;

        Error_Code = Process_Mng.Get_Command_Line(Command_Line_Value);

        if (Error_Code != ERROR_SUCCESS)
            return Error_Code;

        wchar_t Buf[1024];

        swprintf_s(
            Buf,
            _countof(Buf),
            L"[%04d/%02d/%02d %02d:%02d:%02d.%03d] error code [%lld] app name %s file name %s line %s version %s\n",
            Current_Time.tm_year + 1900, Current_Time.tm_mon + 1, Current_Time.tm_mday,
            Current_Time.tm_hour, Current_Time.tm_min, Current_Time.tm_sec, Milli_Sec,
            error,
            (wchar_t*)Command_Line_Value.Get_At(COMMAND_LINE_ASSIGNMENT_NUMBER_CALL_APP_NAME),
            (wchar_t*)Command_Line_Value.Get_At(COMMAND_LINE_ASSIGNMENT_NUMBER_FILE_NAME),
            (wchar_t*)Command_Line_Value.Get_At(COMMAND_LINE_ASSIGNMENT_NUMBER_LINE_NUMBER),
            (wchar_t*)Command_Line_Value.Get_At(COMMAND_LINE_ASSIGNMENT_NUMBER_VERSION)
        );

        StringAssign(Log_Data, Buf);

        return ERROR_SUCCESS;
    }

    public:

        string Day_Base_Root;

        SYSTEM_LOG_MANAGER() {

            StringAssign(Day_Base_Root, NULL_STRING);

            Day_Base_Root.append(MAKER_FOLDER_DRC());
            Day_Base_Root.append(ITEM_FOLDER_DRC());
            Day_Base_Root.append(ITEM_LOG_FOLDER_DRC());
            Day_Base_Root.append(ITEM_SYSTEM_LOG_FOLDER_DRC());
            Day_Base_Root.append(ITEM_SYSTEM_DAY_LOG_FOLDER_DRC());
        }

        virtual ulong Get_Main_Directory(string& result) override {

            PWSTR path = nullptr;

            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path))) {

                StringAssign(result, path);
                CoTaskMemFree(path);

                return ERROR_SUCCESS;
            }

            else {

                SYSTEM_ERROR_VALUE::Set_System_Error(

                    CALC_CUSTOM_ERROR_CODE(CUSTOM_ERROR_CODE_FAILED_GET_LOCAL_APP_DRC),
                    CALC_CUSTOM_ERROR_CODE(CUSTOM_ERROR_CODE_FAILED_GET_LOCAL_APP_DRC),
                    __LINE__,
                    __FILEW__
                );

                return CALC_CUSTOM_ERROR_CODE(CUSTOM_ERROR_CODE_FAILED_GET_LOCAL_APP_DRC);
            }
        }
};

class COMPOSITION_LOG_FLOW : public FLOW_FILE_CONTROLLER {

public:

    inline ulong Write_System_Log(const ulong& boot_time, SYSTEM_LOG_MANAGER& file_mng) {

        
        string File_Name;
        string File_Drc;

        StringAssign(File_Name, NULL_STRING);
        StringAssign(File_Drc, NULL_STRING);

        ulong Error_Code = file_mng.Get_Log_File_Name(File_Name);

        Error_Code = Common_Creaate_Root(file_mng, File_Name, File_Drc);

        if (Error_Code != ERROR_SUCCESS)
            return Error_Code;

        string Main_Drc;

        Error_Code = file_mng.Get_Main_Directory(Main_Drc);

        if (Error_Code != ERROR_SUCCESS)
            return Error_Code;

        Main_Drc.append(file_mng.Day_Base_Root);

        file_mng.Maintenance_Log_Capacity(Main_Drc);

        BYTE8_FILE_MANAGER File_M;

        ulong Error = 0, Vis = 0, Last_Time = 0;

        Error_Code = Byte8_Read(Error, Last_Time, ITEM_ERR_MS_ERR_FILE_NAME(), File_M);

        if (Error_Code != ERROR_SUCCESS)
            return Error_Code;

        File_M.Lock_Mng.Unlock_File(File_M.File_Handle.get());

        if (boot_time < Last_Time) {

            SYSTEM_ERROR_VALUE::Set_System_Error(

                CALC_CUSTOM_ERROR_CODE(CUSTOM_ERROR_CODE_FAILED_GET_FILE_TIME),
                _GetWin32LastError(),
                __LINE__,
                __FILEW__
            );

            return CALC_CUSTOM_ERROR_CODE(CUSTOM_ERROR_CODE_FAILED_GET_FILE_TIME);
        }

        Error_Code = Byte8_Read(Vis, Last_Time, ITEM_ERR_MS_VISIBLE_ERR_NUM_FILE_NAME(), File_M);

        if (Error_Code != ERROR_SUCCESS)
            return Error_Code;

        File_M.Lock_Mng.Unlock_File(File_M.File_Handle.get());

        if (boot_time < Last_Time) {

            SYSTEM_ERROR_VALUE::Set_System_Error(

                CALC_CUSTOM_ERROR_CODE(CUSTOM_ERROR_CODE_FAILED_GET_FILE_TIME),
                _GetWin32LastError(),
                __LINE__,
                __FILEW__
            );

            return CALC_CUSTOM_ERROR_CODE(CUSTOM_ERROR_CODE_FAILED_GET_FILE_TIME);
        }

        ulong Read_Error_Code = Error;

        if (Vis != ERROR_SUCCESS)
            Read_Error_Code = Vis;

        Error_Code = Common_Log_Flow(File_Drc, Read_Error_Code, DUMMY_VALUE, DUMMY_VALUE, file_mng);

        if (Error_Code != ERROR_SUCCESS)
            return Error_Code;

        return ERROR_SUCCESS;
    }
};


int WINAPI WinMain(

    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow) {

    FILETIME Curret_Time = {};

    GetSystemTimeAsFileTime(&Curret_Time);

    ulong Boot_Time = ((ulong)Curret_Time.dwHighDateTime << 32) | Curret_Time.dwLowDateTime;
    

    PROCESS_MANAGER Process_Mng;

    if (!Process_Mng.Trial_Get_Mutex(MUTEX_LOCAL_NAME_SYSTEM_LOG_PROCESS())) {

        return 0;
    }

    SYSTEM_LOG_MANAGER Log_Mng;
    COMPOSITION_LOG_FLOW File_Flow;

    ulong Error_Code = File_Flow.Write_System_Log(Boot_Time, Log_Mng);

    return 0;
}