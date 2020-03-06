// $Id: OmrpcMonitor.java,v 1.1.1.1 2004-11-03 21:01:38 yoshihiro Exp $

/* graphic display of mpstat output */
import java.lang.*;
import java.net.*;
import java.io.*;
import java.applet.Applet;
import java.awt.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import javax.swing.*;

final public class OmrpcMonitor extends java.applet.Applet
{
    OmrpcMonitorBars[] panels;
    OmrpcMonitorUDPcon udpcon;
    Hashtable ht;
    int cols = 5;
    int rows = 4;
    int cellw = 130;
    int cellh = 40;
    static int nodes;
    int maxNodes;
    
    public void init(){
	rows = Integer.parseInt(getParameter("rows"));
	cols = Integer.parseInt(getParameter("cols"));
	cellw = Integer.parseInt(getParameter("cellw"));
	cellh = Integer.parseInt(getParameter("cellh"));
	makeBars();
    }

    public void barsInit(){
	ht = new Hashtable();
    }

    void makeBars(){
	while(maxNodes > rows*cols) rows++;
	setLayout(new GridLayout(rows, cols));
	panels = new OmrpcMonitorBars[maxNodes];
	int i = 0;
	CELLS:
	for(int r = 0; r < rows; r++){
	    for(int c = 0; c < cols; c++){
		OmrpcMonitorBars panel 
		    = new OmrpcMonitorBars(null, null,cellw, cellh);
		panel.setVisible(false);
		add(panel);
		panels[i] = panel;
		i++;
		if(i >= maxNodes)
		    break CELLS;
	    }
	}
    }

    OmrpcMonitorBars getOmrpcBars(String hostname, String agent){
	OmrpcMonitorBars ob;

	ob = (OmrpcMonitorBars)ht.get(hostname);
	if(ob == null){
	    if(nodes < maxNodes){
	    ob = panels[nodes++];
	    ob.setproperty(hostname,agent);
	    ht.put(hostname,ob);
	    }else{
		return null;
	    }
	}
	System.err.println("ob"+ob);
	return ob;
    }

    public void start() {
	/*System.out.println("applet start...");*/
	//	for ( int i = 0 ; i < maxNodes ; i++ ) {
	//	    panels[i].start();
	//	}
	udpcon = new OmrpcMonitorUDPcon(this);
	udpcon.listen();
    }


    public void stop() {
	/*System.out.println("applet stop");*/
	//	for ( int i = 0 ; i < maxNodes ; i++ ) {
	//	    panels[i].stop();
	//	}
    }

    public void destroy() {
	/*System.out.println("applet destroy");*/
    }

    //    public void paint() {
	/*System.out.println("applet paint");*/
    //    }

    public static void main(String[] args) {
	/* mpstat(cols rows cellw cellh hosts...) */
	Frame f = new Frame("OmniRPC Load Monitor");
	OmrpcMonitor stat = new OmrpcMonitor();

	String hosts_args = "";
	for(int ac = 0; ac < args.length; ac++){
	    String arg = args[ac];
	    if(arg.equals("-cols")) stat.cols = Integer.parseInt(args[++ac]);
	    else if(arg.equals("-rows")) stat.rows = Integer.parseInt(args[++ac]);
	    else if(arg.equals("-cellw")) stat.cellw = Integer.parseInt(args[++ac]);
	    else if(arg.equals("-cellh")) stat.cellh = Integer.parseInt(args[++ac]);
	    else if(arg.equals("-nodes")) stat.maxNodes = Integer.parseInt(args[++ac]);
	    else if(arg.equals("-help")) Usage();
	    else hosts_args += " "+arg;
	}
	stat.barsInit();
	stat.maxNodes= stat.rows*stat.cols;
	nodes = 0;

	stat.makeBars();

	f.add(stat, "Center");
	f.pack();
	f.addWindowListener(new WindowAdapter(){
		public void windowClosing(WindowEvent e){
		    System.exit(0);
		}
	    });
	f.show();
	stat.start();
    }
    
    static void Usage(){
	System.err.println("omrpcstat moninor program: jmpstat options hosts ...");
	System.err.println("if trun is runing, monitor using trun 5555");
	System.err.println("otherwise, monior using 'rsh hosts mpstat 1'");
	System.err.println(" options:");
	System.err.println("  -cols <n> : number of columns in cell (default 1)");
	System.err.println("  -rows <n> : number of rows in cell (default 1)");
	System.err.println("  -cellw <n> : cell width in pixels (default 150)");
	System.err.println("  -cellh <n> : cell hight (default 140)");
	System.err.println("  -nodes <n> : number of nodes (default 10)");
	System.err.println("  -help: display this message");
	System.exit(0);
    }
}



final class OmrpcMonitorBars extends java.awt.Canvas{
    static int x = 20;
    static int y = 5 + 25;
    static int w = 100;
    static int h = 15;
    
    Thread thread;
    int running = 0;
    String host;
    String agent;

    int load;

    public OmrpcMonitorBars(String host, String agent, int w, int h) {
	super();
	this.host = host;
	this.agent = agent;
	setSize(w, h);
    }

    synchronized void setproperty(String host, String agent) {
	this.host = host;
	this.agent = agent;
	this.setVisible(true);
    }

    void updateGraph(int load) {
	this.load = load;
	repaint();
    }
    
    public void update(Graphics g) {
	paint(g);
    }

    public void paint(Graphics g) {
	drawGraph(g);
    }

    synchronized void drawGraph(Graphics g) {

	g.setColor(Color.black);
	g.drawString(host, 20, 20);
	Graphics2D g2 = (Graphics2D) g;
	/* change graph's color with load value. */
	if(load > 200){

	    g2.setColor(Color.red);
	    /* max load is 2 */
	    load = 100;
	}else{
	    /* background */
	    
	    g2.fillRect(x+load, y, w-load, h);
	    GradientPaint redtowhite 
		= new GradientPaint(x,y,Color.blue, x+w, y, Color.red);
	    g2.setPaint(redtowhite);
	}
	/* draw graph */
	g2.fill(new RoundRectangle2D.Double(x, y, load, h, 0, 0));	
    }
}



final class OmrpcMonitorUDPcon {
    OmrpcMonitor parent;
    OmrpcMonitorBars pbar;

    DataOutputStream outs;
    DataInputStream ins;
    DatagramSocket socket;
    final int port = 5555;
    final int MAX_LENGTH = 2048;
    byte[] data;
    DatagramPacket packet;

    public OmrpcMonitorUDPcon(OmrpcMonitor parent) {
	this.parent = parent;
	data = new byte[MAX_LENGTH];
	try {
            socket = new DatagramSocket(port);
	    packet  = new DatagramPacket( data, MAX_LENGTH );
        }
        catch( SocketException e ) {
            System.err.println( "Socket Error" );
            System.exit(-1);
        }
        catch( IOException e ) {
            System.err.println( "IO Error" );
            System.exit(-1);
        }
    }
    
    public void listen(){
	double load;
	InetAddress  inet;
	String hostname;
	String agent;
	OmrpcMonitorBars pbars;

	try {
            while( true ) {
                socket.receive( packet );
                data = packet.getData();
		inet = packet.getAddress();
		int length = packet.getLength();
                String message = new String( data, 0, length );
		
		agent = inet.getCanonicalHostName();
		System.err.println("agent:"+agent
				   +", addr:"+inet.getHostAddress());
		StringTokenizer st = new StringTokenizer(message);
		/* parse input string*/
		if(st.hasMoreTokens()){
		    hostname = st.nextToken();
		    System.err.println("hostname:"+ hostname );
		    load = Double.parseDouble(st.nextToken());
		    System.err.println("load:" + load);
		    pbars = parent.getOmrpcBars(hostname,agent);
		    if(pbars == null){
			continue;
		    }
		    pbars.updateGraph((int)(load*100));
		    parent.repaint();
		}
		/* search  and update */
            }
        }
        catch( SocketException e ) {
            System.err.println( "Socket Error" );
            System.exit(-1);
        }
        catch( IOException e ) {
            System.err.println( "IO Error" );
            System.exit(-1);
        }
    }
}
