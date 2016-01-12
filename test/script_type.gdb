# -*- mode: gdb-script -*-

set verbose off
set confirm off


break user_process_fork
commands
	printf "pid of current proceess is : "
	print current_process->PID
	continue
	printf "pid of current proceess is : "
	print current_process->PID
end
	
break syscall.c:296
commands
	print current_process->PID

	if $1 != $2 
		printf "test OK\n"
	else 
		printf "test Fail\n"
	end 

end


target remote:1234
continue
