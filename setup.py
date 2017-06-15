import cx_Freeze

executables = [cx_Freeze.Executable("midi.py")]

cx_Freeze.setup(
        name="Midi Analyzer",
        options={"build_exe":{"packages":["pygame"], "include_files":["freesansbold.ttf"]}},
        executables = executables
        )
