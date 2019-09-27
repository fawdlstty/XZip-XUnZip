#ifndef __X_ZIP_UNZIP_HPP__
#define __X_ZIP_UNZIP_HPP__

#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <cstdint>
#include <Windows.h>

#include "../FawLib/include/FawLib/FawLib.hpp"



#include "XZip.h"
#include "XUnzip.h"
//#ifndef XUNZIP_H
//DECLARE_HANDLE (HZIP);
//#endif
//typedef DWORD ZRESULT;
//
//#define ZIP_HANDLE   1
//#define ZIP_FILENAME 2
//#define ZIP_MEMORY   3
//#define ZIP_FOLDER   4
//
//
//// These are the result codes:
//#define ZR_OK         0x00000000     // nb. the pseudo-code zr-recent is never returned,
//#define ZR_RECENT     0x00000001     // but can be passed to FormatZipMessage.
//// The following come from general system stuff (e.g. files not openable)
//#define ZR_GENMASK    0x0000FF00
//#define ZR_NODUPH     0x00000100     // couldn't duplicate the handle
//#define ZR_NOFILE     0x00000200     // couldn't create/open the file
//#define ZR_NOALLOC    0x00000300     // failed to allocate some resource
//#define ZR_WRITE      0x00000400     // a general error writing to the file
//#define ZR_NOTFOUND   0x00000500     // couldn't find that file in the zip
//#define ZR_MORE       0x00000600     // there's still more data to be unzipped
//#define ZR_CORRUPT    0x00000700     // the zipfile is corrupt or not a zipfile
//#define ZR_READ       0x00000800     // a general error reading the file
//// The following come from mistakes on the part of the caller
//#define ZR_CALLERMASK 0x00FF0000
//#define ZR_ARGS       0x00010000     // general mistake with the arguments
//#define ZR_NOTMMAP    0x00020000     // tried to ZipGetMemory, but that only works on mmap zipfiles, which yours wasn't
//#define ZR_MEMSIZE    0x00030000     // the memory size is too small
//#define ZR_FAILED     0x00040000     // the thing was already failed when you called this function
//#define ZR_ENDED      0x00050000     // the zip creation has already been closed
//#define ZR_MISSIZE    0x00060000     // the indicated input file size turned out mistaken
//#define ZR_PARTIALUNZ 0x00070000     // the file had already been partially unzipped
//#define ZR_ZMODE      0x00080000     // tried to mix creating/opening a zip 
//// The following come from bugs within the zip library itself
//#define ZR_BUGMASK    0xFF000000
//#define ZR_NOTINITED  0x01000000     // initialisation didn't work
//#define ZR_SEEK       0x02000000     // trying to seek in an unseekable file
//#define ZR_NOCHANGE   0x04000000     // changed its mind on storage, but not allowed
//#define ZR_FLATE      0x05000000     // an internal error in the de/inflation code
//
//#define ZIP_SUCCESS(zr)			(zr == ZR_OK)
//
//
//
//HZIP CreateZipZ (void *z, unsigned int len, DWORD flags);
//ZRESULT ZipAdd (HZIP hz, const TCHAR *dstzn, void *src, unsigned int len, DWORD flags);
//ZRESULT CloseZipZ (HZIP hz);
//
//typedef struct {
//	int index;                 // index of this file within the zip
//	char name [MAX_PATH];      // filename within the zip
//	DWORD attr;                // attributes, as in GetFileAttributes.
//	FILETIME atime, ctime, mtime;// access, create, modify filetimes
//	long comp_size;            // sizes of item, compressed and uncompressed. These
//	long unc_size;             // may be -1 if not yet known (e.g. being streamed in)
//} ZIPENTRYA;
//typedef struct {
//	int index;                 // index of this file within the zip
//	TCHAR name [MAX_PATH];     // filename within the zip
//	DWORD attr;                // attributes, as in GetFileAttributes.
//	FILETIME atime, ctime, mtime;// access, create, modify filetimes
//	long comp_size;            // sizes of item, compressed and uncompressed. These
//	long unc_size;             // may be -1 if not yet known (e.g. being streamed in)
//} ZIPENTRYW;
//#ifdef _UNICODE
//#define ZIPENTRY ZIPENTRYW
//#define GetZipItem GetZipItemW
//#define FindZipItem FindZipItemW
//#else
//#define ZIPENTRY ZIPENTRYA
//#define GetZipItem GetZipItemA
//#define FindZipItem FindZipItemA
//#endif
//HZIP OpenZipU (void *z, unsigned int len, DWORD flags);
//ZRESULT GetZipItemA (HZIP hz, int index, ZIPENTRY *ze);
//ZRESULT GetZipItemW (HZIP hz, int index, ZIPENTRYW *ze);
//ZRESULT FindZipItemA (HZIP hz, const TCHAR *name, bool ic, int *index, ZIPENTRY *ze);
//ZRESULT FindZipItemW (HZIP hz, const TCHAR *name, bool ic, int *index, ZIPENTRYW *ze);
//ZRESULT UnzipItem (HZIP hz, int index, void *dst, unsigned int len, DWORD flags);
//ZRESULT CloseZipU (HZIP hz);



class XZip {
public:
	void add_file (std::string zip_path, std::string file_name) {
		m_items.push_back (std::make_tuple (zip_path, file_name));
	}

	void add_mem_file (std::string zip_path, void *data, int len) {
		m_items.push_back (std::make_tuple (zip_path, std::string_view ((const char*) data, len)));
	}

	bool save (std::string _filename) {
		HZIP hZip = CreateZipZ (&_filename [0], 0, ZIP_FILENAME);
		std::vector<faw::String> _vpaths;
		for (auto p : m_items) {
			faw::String _dest = std::get<0> (p);
			_dest.replace_self (_T ('/'), _T ('\\'));
			if (_dest.find (_T ('\\')) != faw::String::_npos) {
				faw::String _dest_path = _dest.substr (0, _dest.find (_T ('\\')));
				size_t i = 0;
				for (; i < _vpaths.size (); ++i) {
					if (_vpaths [i] == _dest_path)
						break;
				}
				if (i == _vpaths.size ()) {
					if (!ZIP_SUCCESS (ZipAdd (hZip, _dest_path.c_str (), nullptr, 0, ZIP_FOLDER))) {
						CloseZipZ (hZip);
						return false;
					}
					_vpaths.push_back (_dest_path);
				}
			}
			if (std::get<1> (p).index () == 0) {
				std::string _src_path = std::get<0> (std::get<1> (p));
				if (!ZIP_SUCCESS (ZipAdd (hZip, _dest.c_str (), &_src_path [0], 0, ZIP_FILENAME))) {
					CloseZipZ (hZip);
					return false;
				}
			} else {
				std::string_view _src_data = std::get<1> (std::get<1> (p));
				if (!ZIP_SUCCESS (ZipAdd (hZip, _dest.c_str (), const_cast<char*> (_src_data.data ()), _src_data.size (), ZIP_MEMORY))) {
					CloseZipZ (hZip);
					return false;
				}
			}
		}
		return ZIP_SUCCESS (CloseZipZ (hZip));
	}

	//static bool zip_folder (faw::String zip_path, faw::String src_path, std::string save_to) {
	//	HZIP hZip = CreateZipZ (&save_to [0], 0, ZIP_FILENAME);
	//	faw::String _str = "zegolog";
	//	if (!ZIP_SUCCESS (AddFolderContent (hZip, &zip_path [0], &src_path [0], &_str [0]))) {
	//		CloseZipZ (hZip);
	//		return false;
	//	}
	//	return ZIP_SUCCESS (CloseZipZ (hZip));
	//}

private:
	std::vector<std::tuple<std::string, std::variant<std::string, std::string_view>>> m_items;
};

class XUnzip {
public:
	XUnzip (std::string _zipfile) {
		hZip = OpenZipU (&_zipfile [0], 0, ZIP_FILENAME);
	}
	XUnzip (void *data, int len) {
		hZip = OpenZipU (data, len, ZIP_MEMORY);
	}
	~XUnzip () {
		if (hZip) {
			CloseZipU (hZip);
			hZip = 0;
		}
	}

	int get_file_count () {
		ZIPENTRY ze = { 0 };
		GetZipItem (hZip, -1, &ze);
		return ze.index;
	}

	ZIPENTRY get_file_item_info (int _index) {
		ZIPENTRY ze = { 0 };
		GetZipItem (hZip, _index, &ze);
		return ze;
	}

	bool unzip_file_item (int _index, std::string _path) {
		if (!ZIP_SUCCESS (UnzipItem (hZip, _index, &_path [0], 0, ZIP_FILENAME)))
			return false;
		::SetFileAttributesA (_path.c_str (), FILE_ATTRIBUTE_NORMAL);
		return true;
	}

	bool unzip_file_item (int _index, std::string _path, std::vector<uint8_t> buf) {
		return ZIP_SUCCESS (UnzipItem (hZip, _index, &buf [0], buf.size (), ZIP_MEMORY));
	}

private:
	HZIP hZip = 0;
};

#endif //__X_ZIP_UNZIP_HPP__
