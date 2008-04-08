require File.dirname(__FILE__) + '/../spec_helper'
require 'ostruct'
include Controllers

describe Controller, 'start' do
  before do
    @controller = Controller.new(:address              => '0.0.0.0',
                                 :port                 => 3000,
                                 :pid                  => 'thin.pid',
                                 :log                  => 'thin.log',
                                 :timeout              => 60,
                                 :max_conns            => 2000,
                                 :max_persistent_conns => 1000)
    
    @server = OpenStruct.new
    @adapter = OpenStruct.new
    
    Server.should_receive(:new).with('0.0.0.0', 3000).and_return(@server)
    @server.should_receive(:config)
    Rack::Adapter::Rails.stub!(:new).and_return(@adapter)
  end
  
  it "should configure server" do
    @controller.start
    
    @server.app.should == @adapter
    @server.pid_file.should == 'thin.pid'
    @server.log_file.should == 'thin.log'
    @server.maximum_connections.should == 2000
    @server.maximum_persistent_connections.should == 1000
  end
  
  it "should start as daemon" do
    @controller.options[:daemonize] = true
    @controller.options[:user] = true
    @controller.options[:group] = true
    
    @server.should_receive(:daemonize)
    @server.should_receive(:change_privilege)

    @controller.start
  end
  
  it "should configure Rails adapter" do
    Rack::Adapter::Rails.should_receive(:new).with(@controller.options.merge(:root => nil))
    
    @controller.start
  end
  
  it "should mount app under :prefix" do
    @controller.options[:prefix] = '/app'
    @controller.start
    
    @server.app.class.should == Rack::URLMap
  end

  it "should mount Stats adapter under :stats" do
    @controller.options[:stats] = '/stats'
    @controller.start
    
    @server.app.class.should == Stats::Adapter
  end
  
  it "should load app from Rack config" do
    @controller.options[:rackup] = 'example/config.ru'    
    @controller.start
    
    @server.app.class.should == Proc
  end
end

describe Controller do
  before do
    @controller = Controller.new(:pid => 'thin.pid', :timeout => 10)
    @controller.stub!(:wait_for_file)
  end
  
  it "should stop" do
    Server.should_receive(:kill).with('thin.pid', 10)
    @controller.stop
  end
  
  it "should restart" do
    Server.should_receive(:restart).with('thin.pid')
    @controller.restart
  end
  
  it "should write configuration file" do
    silence_stream(STDOUT) do
      Controller.new(:config => 'test.yml', :port => 5000, :address => '127.0.0.1').config
    end

    File.read('test.yml').should include('port: 5000', 'address: 127.0.0.1')
    File.read('test.yml').should_not include('config: ')

    File.delete('test.yml')
  end
end