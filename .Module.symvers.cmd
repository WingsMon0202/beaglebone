cmd_/home/wings/beaglebone/Module.symvers := sed 's/ko$$/o/' /home/wings/beaglebone/modules.order | scripts/mod/modpost -m -a   -o /home/wings/beaglebone/Module.symvers -e -i Module.symvers   -T -
