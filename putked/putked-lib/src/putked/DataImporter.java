package putked;

public interface DataImporter
{
	String getName();
	boolean importTo(String path);
}
