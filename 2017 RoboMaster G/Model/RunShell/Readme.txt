Run.sh will run the Run.out execute file. (camgrab.out -> Run.out)

"-autorun" flag use to autostart.

Change Run.sh properties to "Me/Group(ubuntu)/Others: Read and Write" and enable "Allow executing file as program"

New terminal -> gnome-session-properties -> Startup Applications -> Add ->
-> bash $(AbsPath)/Run.sh -autorun
