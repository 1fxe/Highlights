package dev.fxe.highlights;

import java.util.List;

public final class Highlight {
	private final String id;
	private final boolean userInterest;
	private final NVGSDK_HighlightType highlightTag;
	private final NVGSDK_HighlightSignificance significance;
	private final List<LocalizedPair> nameTable;

	/***
	 *
	 * @param id  Unique id for game event
	 * @param userInterest whether the player is interested in this event
	 * @param highlightTag Tags for this highlight
	 * @param significance How significant the highlight is
	 * @param nameTable An array of locale-highlightName pairs for the user-facing highlight name. If no names are given, the highlight id will be used as the "name"
	 */
	public Highlight(String id, boolean userInterest, NVGSDK_HighlightType highlightTag, NVGSDK_HighlightSignificance significance, List<LocalizedPair> nameTable) {
		this.id = id;
		this.userInterest = userInterest;
		this.highlightTag = highlightTag;
		this.significance = significance;
		this.nameTable = nameTable;
	}

	public String id() {
		return id;
	}

	public boolean userInterest() {
		return userInterest;
	}

	public NVGSDK_HighlightType highlightTag() {
		return highlightTag;
	}

	public NVGSDK_HighlightSignificance significance() {
		return significance;
	}

	public List<LocalizedPair> nameTable() {
		return nameTable;
	}
}
