#pragma once
#include <stdafx.h>
#include <NxlUtils/NxlPath.hpp>

class SpawnContext
{
public:
	static SpawnContext &Current() { return s_current; }
	static const SpawnContext &Previous() { return s_previous; }
	const char *OnAddingFileArgument(const char *file);
	void OnBeforeSpawning(const char *cmd);
	void OnAfterSpawn(int pid);
	inline int pid()const {
		return _pid;
	}
	inline bool injected() const { return _injected; }
	bool wait_finish() const;
	bool IsAlive() const;
private:
	SpawnContext() :_pid(-1), _injected(false) {
	}
	static SpawnContext s_current;
	static SpawnContext s_previous;
	std::string _cmd;
	int _pid;
	bool _injected;
	std::vector<NxlPath> _fileArgs;
	std::vector<std::pair<NxlPath, NxlPath>> _fileMappings;
};
