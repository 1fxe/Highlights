package dev.fxe.highlights;

public enum NVGSDK_HighlightType {

	/*!
	 * No tags are attached to the highlight
	 */
	NVGSDK_HIGHLIGHT_TYPE_NONE(0x00000000),
	/*!
	 * Signifies direct progress towards completing the game. (e.g. completing a level)
	 */
	NVGSDK_HIGHLIGHT_TYPE_MILESTONE(0x00000001),
	/*!
	 * Player accomplishment of extra challenges independent of progress in completing a game.
	 * (e.g. perfect score in a level), found the easter egg)
	 */
	NVGSDK_HIGHLIGHT_TYPE_ACHIEVEMENT(0x00000002),
	/*!
	 * Event not significant in competion of game. (e.g. player kills a minion)
	 */
	NVGSDK_HIGHLIGHT_TYPE_INCIDENT(0x00000004),
	/*!
	 * Change in player state), triggered by player or externally by game.
	 * (e.g. equipping a weapon)
	 */
	NVGSDK_HIGHLIGHT_TYPE_STATE_CHANGE(0x00000008),
	/*!
	 * Special highlight type that does not notify the user as it happens), to avoid giving
	 * the user a competetive advantage. e.g. A game doesn't tell the user that they recorded
	 * a kill), but wants the highlight later for the user after the round is over.
	 */
	NVGSDK_HIGHLIGHT_TYPE_UNANNOUNCED(0x00000010),
	//! Invalid default value
	NVGSDK_HIGHLIGHT_TYPE_MAX(0x00000020);
	
	private final int value;

	NVGSDK_HighlightType(int value) {
		this.value  = value;
	}

	public int value() {
		return value;
	}
}
