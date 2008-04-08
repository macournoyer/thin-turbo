require File.dirname(__FILE__) + '/../spec_helper'

describe Request, 'processing' do
  it 'should parse in chunks' do
    request = Request.new
    request.parse("POST / HTTP/1.1\r\n").should be_false
    request.parse("Host: localhost\r\n").should be_false
    request.parse("Content-Length: 9\r\n").should be_false
    request.parse("\r\nvery ").should be_false
    request.parse("cool").should be_true

    request.env['CONTENT_LENGTH'].should == '9'
    request.body.read.should == 'very cool'
    request.should validate_with_lint
  end

  it "should move body to tempfile when too big" do
    request = Request.new
    request.parse("POST /postit HTTP/1.1\r\nContent-Length: #{Request::MAX_BODY*2}\r\n\r\n#{'X' * Request::MAX_BODY}")
    request.parse('X' * Request::MAX_BODY)

    request.body.size.should == Request::MAX_BODY * 2
    request.should be_finished
    request.body.class.should == Tempfile
  end

  it "should delete body tempfile when closing" do
    body = 'X' * (Request::MAX_BODY + 1)

    request = R(<<-EOS.chomp, true)
POST /postit HTTP/1.1
Content-Length: #{body.size}

#{body}
EOS

    request.body.path.should_not be_nil
    request.close
    request.body.path.should be_nil
  end

  it "should raise error when header is too big" do
    big_headers = "X-Test: X\r\n" * (1024 * (80 + 32))
    proc { R("GET / HTTP/1.1\r\n#{big_headers}\r\n") }.should raise_error(InvalidRequest)
  end
end