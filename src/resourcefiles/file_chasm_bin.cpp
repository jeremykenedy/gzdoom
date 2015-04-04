/*
** file_chasm_bin.cpp
**
**---------------------------------------------------------------------------
** Copyright 2013 Alexey Lysiuk
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include "resourcefile.h"

#include <cstdio>

#include "cmdlib.h"
#include "doomdef.h"
#include "templates.h"
#include "w_wad.h"


//==========================================================================
//
// Internal definitions
//
//==========================================================================

namespace
{
#pragma pack(1)

	struct ChasmBinHeader
	{
		DWORD magic;
		WORD entryCount;
	};

	struct ChasmBinEntry
	{
		// Pascal string:
		// 1 byte for size and 12 bytes for name in 8.3 format
		// Will be converted to zero-terminated after load
		char name[13];

		DWORD size;
		DWORD offset;
	};

#pragma pack()

	const DWORD CHASM_BIN_MAGIC = MAKE_ID('C', 'S', 'i', 'd');


#pragma pack(2)

	struct RiffHeader
	{
		DWORD fourCC;
		DWORD size;
	};

	struct FormatHeader
	{
		DWORD fourCC;
		DWORD size;
		WORD  audioFormat;
		WORD  channels;
		DWORD sampleRate;
		DWORD byteRate;
		WORD  blockAlign;
		WORD  bitsPerSample;
	};

	struct DataHeader
	{
		DWORD fourCC;
		DWORD size;
	};

	struct WaveHeader
	{
		RiffHeader   riff;
		DWORD        wave;
		FormatHeader format;
		DataHeader   data;
	};

#pragma pack()

	static const DWORD FORMAT_HEADER_SIZE = static_cast<DWORD>(sizeof(FormatHeader) - 8);

	
	class MemoryLump : public FResourceLump
	{
	public:
		explicit MemoryLump(const size_t length)
		{
			Init(NULL, length);
		}

		MemoryLump(const char* const buffer, const size_t length)
		{
			Init(buffer, length);
		}

		~MemoryLump()
		{
			delete m_reader;
			delete m_buffer;
		}

		virtual FileReader* GetReader()
		{
			return m_reader;
		}

		virtual int FillCache()
		{
			Cache    = const_cast<char*>(m_reader->GetBuffer());
			RefCount = -1;

			return -1;
		}

		virtual int GetFileOffset() { return 0; }

		char* GetBuffer()
		{
			return m_buffer;
		}

	private:
		char*         m_buffer;
		MemoryReader* m_reader;

		void Init(const char* const buffer, const size_t length)
		{
			assert(length > 0);

			m_buffer = new char[length];
			m_reader = new MemoryReader(m_buffer, static_cast<long>(length));

			if (NULL != buffer)
			{
				memcpy(m_buffer, buffer, length);
			}

			LumpSize = static_cast<int>(length);
		}

		// Without implementation
		MemoryLump(const MemoryLump&);
		MemoryLump& operator=(const MemoryLump&);

	};

	bool HasExtension(const char* const filename, const char* const extension)
	{
		assert(NULL != filename);
		assert(NULL != extension);

		const char* const extensionToCheck = strrchr(filename, '.');

		return NULL == extension
			? false
			: (0 == strcmp(extension, extensionToCheck + 1));
	}

	// Target must be at least 9 bytes for name and terminating zero
	// just like FResourceLump::Name

	void CopyBaseName(char* const target, const char* const source)
	{
		assert(NULL != target);
		assert(NULL != source);

		memset(target, 0, 9);

		const char* const extension = strrchr(source, '.');

		const size_t charsToCopy = NULL == extension
			? 8  // Weird, copy up to 8 characters
			: extension - source;

		strncpy(target, source, charsToCopy);
	}

}


//==========================================================================
//
// Chasm BIN file
//
//==========================================================================

class FChasmBinFile : public FResourceFile
{
public:
	FChasmBinFile(const char * filename, FileReader *file);
	virtual ~FChasmBinFile();

	virtual bool Open(bool quiet);

	virtual FResourceLump* GetLump(int no);

private:
	TArray<FResourceLump*> m_lumps;

	FString m_soundInfo;

	bool ReadNextEntry(const DWORD index);

	FResourceLump* MakeRawSoundLump(const ChasmBinEntry& entry);
	FResourceLump* MakeWaveSoundLump(const ChasmBinEntry& entry);
	FResourceLump* MakeUncompressedLump(const ChasmBinEntry& entry);

	void AddEntryToSoundInfo(const ChasmBinEntry& entry);
	void AddSoundInfoLump();

	void DumpFiles(const char* const path);

};


//==========================================================================
//
// FChasmBinFile::FChasmBinFile
//
// Initializes a Chasm BIN file
//
//==========================================================================

FChasmBinFile::FChasmBinFile(const char *filename, FileReader *file)
: FResourceFile(filename, file)
{

}

FChasmBinFile::~FChasmBinFile()
{
	for (unsigned int i = 0, count = m_lumps.Size(); i < count; ++i)
	{
		delete m_lumps[i];
	}
}

//==========================================================================
//
// Open it
//
//==========================================================================

bool FChasmBinFile::Open(bool quiet)
{
	ChasmBinHeader header;
	Reader->Read(&header, sizeof header);

	const WORD entryCount = LittleShort(header.entryCount);

	for (DWORD i = 0; i < entryCount; ++i)
	{
		if (!ReadNextEntry(i))
		{
			return false;
		}
	}

	AddSoundInfoLump();

	NumLumps = m_lumps.Size();

	if (!quiet)
	{
		Printf(", %d lumps\n", NumLumps);
	}

	return true;
}

FResourceLump* FChasmBinFile::GetLump(int no)
{
	return static_cast<DWORD>(no) < NumLumps
		? m_lumps[no]
		: NULL;
}

bool FChasmBinFile::ReadNextEntry(const DWORD index)
{
	ChasmBinEntry entry;

	const long bytesRead = Reader->Read(&entry, long(sizeof entry));
	if (size_t(bytesRead) != sizeof entry)
	{
		return false;
	}

	// Convert Pascal string to zero-terminated
	memmove(entry.name, &entry.name[1], sizeof entry.name - 1);
	entry.name[sizeof entry.name - 1] = '\0';

	FResourceLump* lump = NULL;
	bool supportedFormat = false;

	if (   HasExtension(entry.name, "PCM")
		|| HasExtension(entry.name, "RAW")
		|| HasExtension(entry.name, "SFX"))
	{
		lump = MakeRawSoundLump(entry);
		supportedFormat = true;
	}
	else if (HasExtension(entry.name, "WAV"))
	{
		lump = MakeWaveSoundLump(entry);
		supportedFormat = true;
	}
	else
	{
		lump = MakeUncompressedLump(entry);
		supportedFormat = HasExtension(entry.name, "CEL");
	}

	// Set common lump parameters

	lump->Owner    = this;
	lump->LumpSize = LittleLong(entry.size);
	lump->FullName = copystring(entry.name);

	// TODO: namespace

	if (supportedFormat)
	{
		CopyBaseName(lump->Name, entry.name);
	}

	m_lumps.Push(lump);

	return true;
}

FResourceLump* FChasmBinFile::MakeRawSoundLump(const ChasmBinEntry& entry)
{
	const size_t waveFileSize = sizeof(WaveHeader) + entry.size;

	// All RAW audio files are in the same format:
	// 11025 Hz, mono, 8 bits, uncompressed

	const WaveHeader header =
	{
		// RiffHeader
		{
			MAKE_ID('R', 'I', 'F', 'F'),
			static_cast<DWORD>(waveFileSize - sizeof(RiffHeader))
		},
		MAKE_ID('W', 'A', 'V', 'E'),
		// FormatHeader
		{
			MAKE_ID('f', 'm', 't', ' '),
			FORMAT_HEADER_SIZE,
			1,     // PCM audio format
			1,     // channels
			11025, // sample rate
			11025, // byte rate
			1,     // block alignment
			8,     // bits per sample;
		},
		// DataHeader
		{
			MAKE_ID('d', 'a', 't', 'a'),
			entry.size
		}
	};

	MemoryLump* lump = new MemoryLump(waveFileSize);
	lump->Namespace = ns_sounds;

	char* buffer = lump->GetBuffer();
	
	memcpy(buffer, &header, sizeof header);
	buffer += sizeof header;

	const long savedPos = Reader->Tell();

	Reader->Seek(entry.offset, SEEK_SET);
	Reader->Read(buffer, entry.size);
	Reader->Seek(savedPos, SEEK_SET);

	AddEntryToSoundInfo(entry);

	return lump;
}

FResourceLump* FChasmBinFile::MakeWaveSoundLump(const ChasmBinEntry& entry)
{
	const long savedPos = Reader->Tell();
	Reader->Seek(entry.offset, SEEK_SET);

	WaveHeader header;
	Reader->Read(&header, sizeof header);

	// Some wave files have incorrect size in format chunk

	const bool isHeaderBroken = FORMAT_HEADER_SIZE != header.format.size;
	FResourceLump* lump = NULL;

	if (isHeaderBroken)
	{
		header.format.size = FORMAT_HEADER_SIZE;

		MemoryLump* const fixedLump = new MemoryLump(entry.size);
		char* buffer = fixedLump->GetBuffer();

		memcpy(buffer, &header, sizeof header);
		buffer += sizeof header;

		const long bytesToRead = static_cast<long>(entry.size - sizeof header);
		Reader->Read(buffer, bytesToRead);

		lump = fixedLump;
	}
	else
	{
		lump = MakeUncompressedLump(entry);
	}

	lump->Namespace = ns_sounds;

	AddEntryToSoundInfo(entry);

	Reader->Seek(savedPos, SEEK_SET);

	return lump;
}

FResourceLump* FChasmBinFile::MakeUncompressedLump(const ChasmBinEntry& entry)
{
	FUncompressedLump* lump = new FUncompressedLump;

	lump->Position = LittleLong(entry.offset);

	return lump;
}


void FChasmBinFile::AddEntryToSoundInfo(const ChasmBinEntry& entry)
{
	char baseName[9];
	CopyBaseName(baseName, entry.name);

	char soundInfoLine[128];
	mysnprintf(soundInfoLine, sizeof soundInfoLine, "chasm/%s %s\n", baseName, baseName);

	m_soundInfo += soundInfoLine;
}

void FChasmBinFile::AddSoundInfoLump()
{
	MemoryLump* lump = new MemoryLump(m_soundInfo.GetChars(), m_soundInfo.Len());
	lump->Owner    = this;
	lump->FullName = copystring("sndinfo.txt");
	
	strcpy(lump->Name, "SNDINFO");

	m_lumps.Push(lump);

	m_soundInfo.Truncate(0);
}

void FChasmBinFile::DumpFiles(const char* const path)
{
	CreatePath(path);

	for (DWORD i = 0, count = m_lumps.Size(); i < count; ++i)
	{
		FResourceLump* const lump = m_lumps[i];

		char filename[1024] = {};
		mysnprintf(filename, sizeof filename, "%s/%s", path, lump->FullName.GetChars());

		FILE* const file = fopen(filename, "wb");
		if (NULL == file)
		{
			continue;
		}

		FileReader* reader = lump->GetReader();
		assert(NULL != reader);

		for (long bytesLeft = lump->LumpSize; bytesLeft > 0; /* EMPTY */)
		{
			unsigned char buffer[4096];

			const long bytesToRead = MIN(bytesLeft, long(sizeof buffer));
			const long bytesRead = reader->Read(buffer, bytesToRead);

			fwrite(buffer, bytesRead, 1, file);

			bytesLeft -= bytesRead;
		}

		fclose(file);
	}
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile *CheckChasmBin(const char *filename, FileReader *file, bool quiet)
{
	if (file->GetLength() > 4)
	{
		DWORD magic;

		file->Seek(0, SEEK_SET);
		file->Read(&magic, 4);
		file->Seek(0, SEEK_SET);

		if (LittleLong(magic) == CHASM_BIN_MAGIC)
		{
			FResourceFile* result = new FChasmBinFile(filename, file);

			if (result->Open(quiet))
			{
				return result;
			}

			result->Reader = NULL; // to avoid destruction of reader
			delete result;
		}
	}

	return NULL;
}
