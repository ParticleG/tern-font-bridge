package io.github.particleg.ternfontbridge;
import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_LoadPackage;
import java.io.FileInputStream;
import java.security.MessageDigest;
public final class MainHook implements IXposedHookLoadPackage {
 public void handleLoadPackage(XC_LoadPackage.LoadPackageParam p) {
  if (!"so.stencil.tern".equals(p.packageName) || !"so.stencil.tern".equals(p.processName)) return;
  try {
   MessageDigest md=MessageDigest.getInstance("SHA-256");
   try(FileInputStream in=new FileInputStream(p.appInfo.sourceDir)) {
    byte[] b=new byte[65536]; int n; while((n=in.read(b))!=-1) md.update(b,0,n);
   }
   StringBuilder s=new StringBuilder(); for(byte b:md.digest()) s.append(String.format("%02x",b&255));
   if(!s.toString().equals("20d357f2a3a3a1e58ed207244f9148fe4ff7236059ed27a111cfb5704d1598aa")) {
    XposedBridge.log("TernFontProbe: APK mismatch; no native load"); return;
   }
   XposedBridge.log("TernFontProbe: APK verified; loading Maple Regular replacement 0.3");
   System.loadLibrary("ternprobe");
  } catch(Throwable t) { XposedBridge.log("TernFontProbe: load failed"); XposedBridge.log(t); }
 }
}