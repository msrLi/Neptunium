# (c) binsonLi add 


all: target
clean: target_clean
help:
	@echo  "target:    build target"
	@echo  "target_clean clean all platform libs"

target:
	@echo compare target 
	make -C target

target_clean:
	make -C target clean

.PHONY:all clean help target target_clean
