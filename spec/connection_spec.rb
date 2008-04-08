require File.dirname(__FILE__) + '/spec_helper'

describe Connection do
  before do
    @connection = Connection.new(mock('EM', :null_object => true))
    @connection.post_init
    @connection.app = proc do |env|
      [200, {}, ['']]
    end
  end
  
  it "should parse on receive_data" do
    @connection.request.should_receive(:parse).with('GET')
    @connection.receive_data('GET')
  end

  it "should close connection on InvalidRequest error in receive_data" do
    @connection.request.stub!(:parse).and_raise(InvalidRequest)
    @connection.should_receive(:close_connection)
    @connection.receive_data('')
  end
  
  it "should process when parsing complete" do
    @connection.request.should_receive(:parse).and_return(true)
    @connection.should_receive(:process)
    @connection.receive_data('GET')
  end
  
  it "should process" do
    @connection.process
  end
  
  it "should return HTTP_X_FORWARDED_FOR as remote_address" do
    @connection.request.env['HTTP_X_FORWARDED_FOR'] = '1.2.3.4'
    @connection.remote_address.should == '1.2.3.4'
  end
  
  it "should return nil on error retreiving remote_address" do
    @connection.stub!(:get_peername).and_raise(RuntimeError)
    @connection.remote_address.should be_nil
  end
  
  it "should return nil on nil get_peername" do
    @connection.stub!(:get_peername).and_return(nil)
    @connection.remote_address.should be_nil
  end
  
  it "should return nil on empty get_peername" do
    @connection.stub!(:get_peername).and_return('')
    @connection.remote_address.should be_nil
  end
  
  it "should return remote_address" do
    @connection.stub!(:get_peername).and_return(Socket.pack_sockaddr_in(3000, '127.0.0.1'))
    @connection.remote_address.should == '127.0.0.1'
  end
  
  it "should not be persistent" do
    @connection.should_not be_persistent
  end

  it "should be persistent when response is and allowed" do
    @connection.response.stub!(:persistent?).and_return(true)
    @connection.can_persist!
    @connection.should be_persistent
  end

  it "should not be persistent when response is but not allowed" do
    @connection.response.persistent!
    @connection.should_not be_persistent
  end
end