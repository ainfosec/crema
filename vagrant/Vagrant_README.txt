This is an Ubuntu 14.04 64-bit Vagrant instance with llvm and clang installed.

You will need to install:

    VirtualBox
    Vagrant

To use this VM for development, copy Vagrantfile and provision.sh to the project's
home folder. Your file structure should look like this:

/project
    Vagrantfile
    provision.sh
    /crema
        README
        contributors.txt
        /docs
        /src
        /vagrant

From the /project folder, type 'vagrant up' to start the VM. Then, type 'vagrant ssh'
to go to the Ubuntu 14.04 terminal. You will be able to access the /crema folder from
the VM. Just type 'cd ../../vagrant'.
