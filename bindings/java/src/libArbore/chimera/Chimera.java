package libArbore.chimera;

import libArbore.network.ChatPacket;
import libArbore.network.Host;
import libArbore.network.Network;
import libArbore.util.Key;

public class Chimera {

	public Host getMe(){
		return new Host(N_getMe(instance));
	}
	
	public Network getNetwork(){
			return new Network(N_getNetwork(instance));
	}
	
	public void join(Host bootstrap)
	{
		N_join(instance, bootstrap.GetInstance());
	}
	
	public boolean route(ChatPacket cp)
	{
		return N_route(instance, cp.GetInstance());
	}
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javachimera");
    }

	public Chimera(Network net, int port, Key k)  {
        instance = initCppSide(net.GetInstance(), port, k.GetInstance());
    }
	
	public void finalize() {
		destroyCppSide(instance);
	}
	
	private native long N_getMe(long instance);
	private native long N_getNetwork(long instance);
	private native void N_join(long instance, long bootstrap);
	private native boolean N_route(long instance, long cp);
	private native long initCppSide(long network, int port, long key);
	private native void destroyCppSide(long instance);
    private long instance;
}