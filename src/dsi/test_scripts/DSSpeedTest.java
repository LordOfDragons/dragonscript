
/**
 * @author khan
 * Java version of the ds speed test.
 */
public class DSSpeedTest {

    public static void main(String[] args) {
        int vValue;
        float vRet;
        long vTime = System.currentTimeMillis();
        for (int i = 0; i < 150000; ++i) {
            vValue = 1145677;
            vRet = (vValue & 2047) / 8.0f;
			vRet = ((vValue >> 11) & 2047) / 8.0f;
			vRet = ((vValue >> 22) & 1023) / 4.0f;
        }
        vTime = System.currentTimeMillis() - vTime;
        System.out.print("Finished Test, "+(vTime / 1000.0f)+"s.");
    }
}
