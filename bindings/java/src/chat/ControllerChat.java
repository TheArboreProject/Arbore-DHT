package chat;

import libArbore.chimera.ChatMessageListener;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.GregorianCalendar;

import javax.swing.JLabel;

import libArbore.chimera.Chimera;
import libArbore.network.ChatPacket;
import libArbore.network.Host;
import libArbore.network.Network;
import libArbore.scheduler.Scheduler;
import libArbore.util.Key;

public class ControllerChat {

	private ChatWindow view;
	private Network network = new Network();
	private Chimera chimera;
	private Key me;

	public ControllerChat(ChatWindow view){
		this.view = view;
		initWindowListener();
		initChimeraListener();

		Key me = Key.GetRandomKey();
		Scheduler.StartSchedulers(5);
		network.Start();

	}

	private void initChimeraListener() {
		chimera.addListener(new ChatMsgListenImpl());
		
	}

	/**
	 * Initialize the listener for the main window
	 */
	private void initWindowListener() {
		view.addOkButtonListener(new OkButtonListener());
		view.addSendButtonListener(new SendButtonListener());
		view.addConnectButtonListener(new ConnectButtonListener());
	}

	class OkButtonListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			view.hidePortField();
			int port = Integer.parseInt(view.getPortField().getText());
			chimera = new Chimera(network, port, me);
				Host bootstrap_host = network.getHost_List().DecodeHost(view.getAdressField().getText());
				System.out.println("Joining host: " + bootstrap_host);
				chimera.join(bootstrap_host);
			}
			
		}

	class SendButtonListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			String msg = view.getTextField().getText();
			int[] list = view.getList().getSelectedIndices();
			for(int i=0; i<list.length; i++)
			{
			ChatPacket pack = new ChatPacket(chimera.getMe().getKey(),
					chimera.getLeafset().getHost(i).getKey(),
					msg);
			chimera.route(pack);
			}
		}
	}

	class ConnectButtonListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			Host bootstrap_host = network.getHost_List().DecodeHost(view.getAdressField().getText());
			System.out.println("Joining host: " + bootstrap_host);
			chimera.join(bootstrap_host);
			view.hideConnectField();
		}
	}
	
	class ChatMsgListenImpl implements ChatMessageListener {

		@Override
		public void MessageReceived(String s, Host h) {
			String fmsg = new String();
			JLabel msg = new JLabel();
			GregorianCalendar now = new GregorianCalendar();
			String hour = String.valueOf(now.getMaximum(GregorianCalendar.HOUR_OF_DAY));
			fmsg = hour + "  - from " + h.toString() + " - " + s;
			msg.setText(fmsg);
			view.getTextAera().add(msg);
		}
	}
}
