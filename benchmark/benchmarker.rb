require 'rubygems'

class Benchmarker
  PORT          = 7000
  ADDRESS       = '0.0.0.0'
  TMP_POST_FILE = "/tmp/bench-post-file"
  # SERVERS       = %w(mongrel ebb-seq ebb-thread thin thin-turbo)
  SERVERS       = %w(ebb-seq ebb-thread thin-turbo)
  
  def initialize(method, range, options={})
    @method     = method
    @range      = range

    @ran        = false
    @keep_alive = options[:keep_alive]
    @requests   = options[:requests] || 5000
  end
  
  def run
    @results = @range.collect do |i|
      run_on_all @method => i
    end.flatten
  end
  
  def to_s(align=0)
    run unless @results
    Result.header(align) + "\n" + @results.map { |r| r.to_s(align) }.join("\n")
  end
  
  class Result
    attr_accessor :server, :concurrency, :req_sec, :failures, :total_mem, :virtual_mem, :real_mem
    
    def initialize(options)
      @concurrency = options[:concurrency] || 1
      @upload      = options[:upload] || 0
      @download    = options[:download] || 0
    end
    
    def self.header(align=0)
      %w(server concurrency upload download req_sec failures total_mem virtual_mem real_mem).map { |i| i.rjust(align)  }.join(", ")
    end
    
    def to_s(align=0)
      [@server, @concurrency, @upload, @download, @req_sec, @failures, @total_mem, @virtual_mem, @real_mem].map { |i| i.to_s.rjust(align)  }.join(", ")
    end
  end
  
  private
    def start_server(name)
      @server = fork do
        exec "ruby #{File.dirname(__FILE__)}/servers/#{name}.rb"
      end
    
      sleep 2
    end
  
    def stop_server
      Process.kill(9, @server)
      Process.wait
    end
  
    def ab(options={})
      File.open(TMP_POST_FILE, 'w') { |f| f << 'X' * options[:upload].to_i } if options[:upload]
      
      params = []
      params << "size=#{options[:download]}" if options[:download]
      params << "wait=#{options[:wait]}"     if options[:wait]
      
      cmd  = "nice -n20 ab "
      cmd << "-c #{options[:concurrency]} " if options[:concurrency]
      cmd << "-p #{TMP_POST_FILE} "         if options[:upload]
      cmd << "-k "                          if @keep_alive
      cmd << "-n #{@requests} "
      cmd << "#{ADDRESS}:#{PORT}/?" + params.join("&")
      cmd << " 2> /dev/null"
      
      out = `#{cmd}`
      
      File.delete(TMP_POST_FILE) if options[:upload]
      
      out
    end
    
    def run_on_all(options={})
      SERVERS.collect do |server|
        run_on server, options
      end
    end
    
    def run_on(server, options={})
      start_server(server)
      out = ab(options)
      mem = mem(@server)
      stop_server
      
      result = Result.new(options)
      result.server   = server
      result.req_sec  = find_field(out, /^Requests.+?(\d+\.\d+)/)
      result.failures = find_field(out, /^Failed requests.+?(\d+)/)
      result.total_mem, result.virtual_mem, result.real_mem = mem
      result
    end
    
    def find_field(out, regex)
      if matches = out.match(regex)
        matches[1].to_i
      else
        0
      end
    end
    
    def mem(pid)
      a = `ps -o vsz,rss -p #{pid}`.split(/\s+/)[-2..-1].map{|el| el.to_i}

      total   = a.first
      real    = a.last
      virtual = total - real

      [total, virtual, real]
    end
end
