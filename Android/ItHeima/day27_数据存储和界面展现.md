
> Eclipse 查看类的继承关系：Ctrl+T


* 引用外部类的对象，可以使用 类名.this  
这就是在 Android 程序中经常使用的，xxActivity.this 作为 context 参数传给内部类的原理

## 关于重复的 id
* 在不同的 layout xml 里，是可以有相同的 id 的。同一个 xml 里 id 不重复即可。
* 相同的 id 在 R.java 里是同一个变量。  
比如 activity\_1.xml 里有个 TextView，其 id 是 android:id="@+id/tv\_hello"  
在 activity\_2.xml 里有个 TextView，其 id 也是 android:id="@+id/tv\_hello"   
他们在 R.java 里都是 public static final int tv_hello=0x7f080000;  
R 的作用仅仅是把字符串翻译成int而已
* findViewById() 可以找到正确的 View，不会因为相同的 id 而冲突
* 原因：
* 首先要知道 View.findViewById() 和 Activity.findViewById() 是两个不同的 API
* 我们平常在 Activity 里使用的 findViewById() 当然都是 Activity.findViewById()
* Activity.findViewById() 实际上先找到了本 Activity 的 View——即setContentView()设置的那个，然后调用的这个View的 View.findViewById()
* 因为最终调用的都是某个 View 的 findViewById()，所以是不会冲突的
* 我们在 ListView 里，使用的是 View.findViewById()。因为我们要做 ListView 的 Adapter 中，把一个 layout xml inflate成一个View，然后在这个View上去按id找东西，所以我们会对 inflate 出来的 View 调用 View.findViewById()。
* 实际上可以理解为，Activity 就是把setContentView()的那个 xml 给 inflate 成了一个 View，然后调用 Activity.findViewById()时，实际上就是调用这个 View 的 View.findViewById()。

## 关于AlertDialog
* 一般使用流程：  
	  
		AlertDialog.Builder builder = new Builder(this); // new 一个 Dialog Builder
		builder.setXXX();								 // 用 builder 设置 dialog 的各种属性
		AlertDialog ad = builder.create();               // 生成 dialog
		ad.show();										// 显示
		// 以上最后两行，也可简化为： builder.show()
* builder.setXXX()  

		builder.setPositiveButton(CharSequence text, OnClickListener l)  // 设置确定按钮  
		builder.setNegativeButton(CharSequence text, OnClickListener l)  // 设置取消按钮
		builder.setTitle(CharSequence text);								// 设置标题
		builder.setSingleChoiceItems(CharSequence[] items, int checkedItem, OnClickListener listener)  //设置单选
		builder.setView(View view)										//设置自定义 View，比如可以弹出一个文本输入框

* 不调用就是没有，比如没有调用setTitle()，则弹出的对话框没有标题。如果没调用setPositiveButton()则弹出的对话框没有确定按钮。
* 如果任何 setXXX() 都没调用，则弹出一个看不见的对话框，效果就是当前 Activity 被灰化，但前台还啥也看不到