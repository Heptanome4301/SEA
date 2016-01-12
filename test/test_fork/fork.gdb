# -*- mode: gdb-script -*-

set verbose off
set confirm off


break kmain-test-fork.c:22
commands
	printf "pid of current proceess is : "
	print current_process->PID
	continue
end
	
break kmain-test-fork.c:25
commands
	printf "pid of current proceess is : "
	print current_process->PID

	if $1 != $2 
		printf "test OK\n"
	else 
		printf "test Fail\n"
	end 

end


target remote:1234
continue
