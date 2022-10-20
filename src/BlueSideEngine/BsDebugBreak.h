namespace Debug {

	enum DebugEvent {
		OnNothing = 0x00,
		OnLoad    = 0x01,
		OnAddRef  = 0x02,
		OnRelease = 0x04,
		OnDelete  = 0x08,

		OnMemory = OnLoad | OnDelete,
		OnAll    = OnLoad | OnAddRef | OnRelease | OnDelete,
	};

	struct DebugInfo {
		char const *  name;
		DebugEvent    showEvent;
		DebugEvent    breakEvent;
	};

	void TriggerEvent( DebugInfo const * infoList, char const * name, int refCount, DebugEvent event );

};
