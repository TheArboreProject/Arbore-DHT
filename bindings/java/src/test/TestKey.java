package test;

import libArbore.util.Key;
import libArbore.util.Log;

public class TestKey {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Key key = new Key();
		Log log = new Log();
		
		log.info(key.toString());
		
		for(int i = 0; i < 10; i++)
		{
			key = Key.GetRandomKey();
			log.info(key.toString());
		}
	}

}
