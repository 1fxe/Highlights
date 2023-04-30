package dev.fxe.highlights;

import java.util.List;

public class Highlights {

	static {
		System.loadLibrary("GfeSDK");
		System.loadLibrary("libMCHighlights");
	}

	public native void init(String gameName, List<Highlight> highlights);

	public native void shutdown();

	public native void tick();

	public native void openGroup(String groupId);

	public native void closeGroup(String groupId, boolean destroy);

	public native void saveScreenShot(String highlightId, String groupId);

	public native void saveVideo(String highLightId, String groupId, int startDelta, int endDelta);

	public native void getNumHighlights(String groupId, NVGSDK_HighlightSignificance sigFilter, NVGSDK_HighlightType tagFilter);

	public native void openSummary(String[] groupIds, int numGroups, NVGSDK_HighlightSignificance sigFilter, NVGSDK_HighlightType tagFilter);

	public native void requestLanguage();

	public native void requestUserSettings();
}