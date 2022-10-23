#pragma once

#include <string>

class CPathEx
{
public:
	CPathEx();
	CPathEx(const wchar_t* path);
	CPathEx& operator = (const wchar_t* path);
	/*!
		* Returns the filename extension, including leading period (.)
		* 
		* \return
		*/
	std::wstring GetExtension() const;
	/*!
		* Returns the path of the parent path 
		* 
		* \return
		*/
	std::wstring GetParentPath() const;
	/*!
		* returns the filename without extension
		* 
		* \return
		*/
	std::wstring GetFileName() const;
	/*!
		* returns the filename with extension
		*
		* \return
		*/
	std::wstring GetFileNameWithExt() const;
	/*!
		* Appends a path, directory separator will be added if necessary
		* 
		* \param path
		* \return
		*/
	CPathEx& operator /= (const wchar_t* path);
	/*!
	* Appends a string
	*
	* \param path
	* \return
	*/
	CPathEx& operator += (const wchar_t* path);
	/*!
	* Returns C-string representation path 
	* 
	* \return
	*/
	const wchar_t* c_str() const;
	/*!
	* Returns string representation path
	*
	* \return
	*/
	std::wstring ToString() const;

public:
	/*!
		* Checks if the given path is an existing file or directory
		* 
		* \param path
		* \return true if the corresponding file or directory exists
		*/
	static bool Exists(const std::wstring& path);
	/*!
		* Checks if the given path is an existing file
		* 
		* \param path
		* \return true if file exists
		*/
	static bool IsFile(const std::wstring& path);
	/*!
		* Checks if the given path is an existing directory
		* 
		* \param path
		* \return true if directory exists
		*/
	static bool IsDir(const std::wstring& path);
	/*!
		* Removes the file
		* 
		* \param path
		* \return true if the file was successfully deleted
		*/
	static bool RemoveFile(const std::wstring& path);
	/*!
		* Creates the directories recursively
		* 
		* \param path
		* \return 0 if successful. Otherwise, error code can be returned
		*/
	static int CreateDirectories(const std::wstring& path);
	/*!
	* Rename the file
	*
	* \param path
	* \return 0 if successful. Otherwise, error code can be returned
	*/
	static int RenameFile(const std::wstring& origFilePath, const std::wstring& newFilePath);

public:
	const static wchar_t PATH_DELIMITER_1 = L'\\';
	const static wchar_t PATH_DELIMITER_2 = L'/';

private:
	std::wstring m_path;

}; // CPathEx

