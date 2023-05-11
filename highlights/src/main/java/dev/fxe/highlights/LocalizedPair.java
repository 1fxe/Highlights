package dev.fxe.highlights;

public final class LocalizedPair {

	private final String localeCode;
	private final String localizedString;

	public LocalizedPair(String localeCode, String localizedString) {
		this.localeCode = localeCode;
		this.localizedString = localizedString;
	}

	public String localeCode() {
		return localeCode;
	}

	public String localizedString() {
		return localizedString;
	}
}
