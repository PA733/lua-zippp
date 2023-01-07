#include "pch.h"

BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}

bool success = true;
static std::string error_msg = "";

// libzippp压缩过程的错误回调函数
static void error_callback(const std::string& message,
	const std::string& strerror,
	int zip_error_code,
	int system_error_code) {
	success = false;
	error_msg = message;
}

extern "C" int extract_zip(lua_State * L) {
	// 获取第一个参数，目标zip文件路径
	const char* zip_filepath = lua_tostring(L, 1);
	// 获取第二个参数，解压缩文件的目标路径
	const char* dest_path = lua_tostring(L, 2);

	success = true;
	error_msg = "";

	libzippp::ZipArchive zip_archive(zip_filepath);
	zip_archive.setErrorHandlerCallback(error_callback);

	// 这里再套一个try catch是因为zip_archive.open()可能会抛出异常
	try {
		// 打开zip压缩包
		zip_archive.open(libzippp::ZipArchive::ReadOnly);
		// 获取文件列表
		auto file_list = zip_archive.getEntries();
		// 遍历文件列表并解压缩所有文件
		for (const auto& entry : file_list) {
			// 解压缩文件的文件名
			std::string filename = entry.getName();
			// 解压缩文件的完整路径
			std::string filepath = std::string(dest_path) + "/" + filename;
			// 解压缩文件
			std::ofstream ofs(filepath, std::ios::binary);
			entry.readContent(ofs);
			ofs.close();
		}
		zip_archive.close();
	}
	catch (const std::exception& ex) {
		success = false;
		error_msg = ex.what();
	}

	lua_pushboolean(L, success);
	lua_pushstring(L, error_msg.c_str());
	return 2;
}

extern "C" int create_zip(lua_State * L) {
	// 获取第一个参数，目标zip文件路径
	const char* zip_filepath = lua_tostring(L, 1);
	// 获取第二个参数，要压缩的文件或文件夹路径
	const char* filepath = lua_tostring(L, 2);

	success = true;
	error_msg = "";

	// 检查目标zip文件是否存在
	// 在确保安全的情况下，此判断可以不需要
	// 如果删去此判断，那么在遇到一个存在的zip文件时，create_zip()会自动向zip内追加文件
	if (std::filesystem::exists(zip_filepath)) {
		// 退出，防止目标zip文件内的文件被覆盖
		success = false;
		error_msg = "the archive already exsits";
		lua_pushboolean(L, success);
		lua_pushstring(L, error_msg.c_str());
		return 2;
	}

	libzippp::ZipArchive zip_archive(zip_filepath);
	zip_archive.setErrorHandlerCallback(error_callback);

	try {
		// 打开目标zip文件
		zip_archive.open(libzippp::ZipArchive::Write);
		// 检测filepath是目录路径还是单文件
		std::filesystem::path fs_path(filepath);
		if (std::filesystem::is_directory(fs_path)) {
			// 如果filepath为目录路径，则递归遍历该文件夹，将其内文件添加到压缩文件中
			std::vector<std::string> file_list = list_files_recursively(filepath);
			for (const std::string& file : file_list) {
				std::string zip_entry_name = file.substr(fs_path.string().size() + 1);
				zip_archive.addFile(zip_entry_name, file);
			}
		}
		else {
			// 如果filepath为单文件路径，则直接将其添加到目标zip文件内
			zip_archive.addFile(fs_path.filename().string(), filepath);
		}

		zip_archive.close();
		success = true;
	}
	catch (const std::exception& ex) {
		success = false;
		error_msg = ex.what();
	}

	lua_pushboolean(L, success);
	lua_pushstring(L, error_msg.c_str());
	return 2;
}

static struct luaL_Reg ls_lib[] = {
	{"extract_zip", extract_zip},
	{"create_zip", create_zip},
	{nullptr, nullptr}
};

extern "C" LUALIB_API int luaopen_zippp(lua_State * L) {
#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, ls_lib);
#else
	luaL_openlib(L, "zippp", ls_lib, 0);
#endif

	return 1;
}


