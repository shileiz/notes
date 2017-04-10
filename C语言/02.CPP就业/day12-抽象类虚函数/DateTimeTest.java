class Date
{
	protected int year, month, day;
	Date(int y, int m, int d) 
	{ 
		SetDate(y, m, d); 
	}
	public void SetDate(int y, int m, int d) 
	{ 
		year = y; 
		month = m; 
		day = d; 
	}
	public void Print() 
	{ 
		System.out.printf("%d/%d/%d;\n",year,month,day); 
	}
}

class DateTime extends Date
{
	DateTime(int y, int m, int d, int h, int mi, int s)
	{ 
		super(y, m, d) ;
		SetTime(h, mi, s); 
	}
	public void SetTime(int h, int mi, int s) 
	{ 
		hours = h;
		minutes = mi;
		seconds = s; 
	}
	public void Print() // 重写父类的方法
	{
		//((Date)this).Print(); // 不行，编译没问题，运行时会报异常。
		super.Print();
		System.out.printf("%d:%d:%d\n",hours,minutes,seconds);
	}
private int hours, minutes, seconds;
}

public class DateTimeTest{
	public static void main(String argv[])
	{
		DateTime dt = new DateTime(2003, 1, 1, 12, 30, 0);
		dt.Print();
	}
}

