# -*- mode: gdb-script -*-

set verbose off
set confirm off

set $started=0

break irq_handler
commands
	printf "current proceess is : "
	print *current_process
	set $started++
	if $started<6
		continue
	end
end



target remote:1234
continue
