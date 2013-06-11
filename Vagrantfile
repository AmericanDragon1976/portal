#!/usr/bin/env ruby

Vagrant.configure("2") do |config|

  config.vm.box     = 'ubuntu_quantal_64'
  config.vm.box_url = 'https://github.com/downloads/roderik/VagrantQuantal64Box/quantal64.box'

  config.vm.provider "virtualbox" do |v|
    v.customize ["modifyvm", :id, "--cpus", "2", "--memory", "1024", "--cpuexecutioncap", "75"]
  end

  config.vm.synced_folder ".", "/vagrant", :nfs => true

  config.vm.provision :chef_solo do |chef|
    chef.add_recipe 'build-essential'
  end

end