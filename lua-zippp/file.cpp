#include "pch.h"

// 获取指定目录下的所有文件的完整路径
// 如果指定的路径是文件，则返回该文件的完整路径
// 如果指定的路径是目录，则返回该目录下所有文件的完整路径
std::vector<std::string> list_files_recursively(const std::string& path) {
	std::vector<std::string> file_list;
	if (std::filesystem::is_regular_file(path)) {
		// 如果指定的路径是文件，则直接将该文件的完整路径加入到文件列表中
		file_list.push_back(path);
	}
	else if (std::filesystem::is_directory(path)) {
		// 如果指定的路径是目录，则遍历该目录下所有文件和子目录
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
			if (entry.is_regular_file()) {
				file_list.push_back(entry.path().string());
			}
		}
	}

	return file_list;
}