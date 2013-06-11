#!/usr/bin/env ruby

Vagrant.configure("2") do |config|

  config.vm.box     = 'pagoda_precise_base'
  config.vm.box_url = 'https://s3.amazonaws.com/vagrant.pagodabox.com/boxes/ubuntu-12.04-x86_64-base.box'

  config.vm.provider "virtualbox" do |v|
    v.customize ["modifyvm", :id, "--cpus", "2", "--memory", "1024", "--cpuexecutioncap", "75"]
  end

  config.vm.synced_folder ".", "/vagrant", :nfs => true

  config.vm.provision :chef_solo do |chef|
    chef.add_recipe 'build-essential'
  end

end