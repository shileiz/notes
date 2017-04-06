* 首先，Text 这种控件默认是不能获取焦点的，所以在 TV 上用遥控器的上下左右键就无法选中它，无法选中也就无法在按下确定键的时候触发 onClick 事件。而像 Button 这种控件默认是可以获取焦点的，当你用上下左右选中一个 Button 之后，按下确定键，就相当于在手机上点了一下，它的 onClick 就触发了。
* 想让 Text 可以获取焦点的方法是在布局文件xml里，给 Text 加个属性：`android:focusable="true"`
* 其次，以上说的都没啥用，因为我们要做的是给 ListView 的 Item 添加点击事件。 这件事情在手机上是正常的，但是到了 TV 上就不行了。
* 首先描述一下出问题的环境：
	* ListView 所在的 Activity 是直接从 ListActivity 继承的
	* 该 Activity 的布局文件里有一个叫 list 的 ListView：


			<?xml version="1.0" encoding="utf-8"?>
			<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
			    android:layout_width="match_parent"
			    android:layout_height="fill_parent"
			    android:orientation="vertical" >
			
			
			    <ListView
			        android:id="@android:id/list"
			        android:layout_width="fill_parent"
			        android:layout_height="0dp"
			        android:layout_marginTop="6dp"
			        android:layout_weight="1" />
			
			    <TextView
			        android:id="@android:id/empty"
			        android:layout_width="fill_parent"
			        android:layout_height="wrap_content"
			        android:text="@string/label_no_data" />
			
			</LinearLayout>

	* ListView 的 Item 是用如下 xml inflate 来的：

			<?xml version="1.0" encoding="utf-8"?>
			
			<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
				android:id="@+id/linearlayout_row"
			    android:orientation="horizontal"
			    android:layout_width="fill_parent"
			    android:layout_height="fill_parent"
			    android:padding="2dp">
				
				<ImageView 
				    android:id="@+id/row_thumbnail"
				    android:layout_width="wrap_content"
				    android:layout_height="wrap_content"
				/>
				
				<TextView
				    android:id="@+id/row_text"
				    android:layout_width="fill_parent"
				    android:layout_height="wrap_content"
				    android:layout_marginLeft="5dp"
				    android:layout_marginStart="5dp"
				    android:layout_gravity="center_vertical"
				/>
			
			</LinearLayout>	

	* onClick 事件的添加：

			// 以下在 Adapter 的 getView 方法里：
			//...
			ImageView img = (ImageView) convertView.findViewById(R.id.row_thumbnail);
			img.setOnClickListener(new OnClickListener() {
				// 处理点击事件
			}
			TextView text = (TextView) convertView.findViewById(R.id.row_text);
			text.setOnClickListener(new OnClickListener() {
				// 处理点击事件
			}

* 问题描述：手机上点击 Item 的 Img 或者 Text 都可以触发事件，但在 TV 上遥控器选中一个 Item 之后点击确定键没反应。
* 问题解决：
* 开始怀疑是因为 Image 和 Text 不能获取焦点，所以给它俩都加了 `android:focusable="true"`， 还是不行
* 后来经过百度（[http://blog.csdn.net/sk719887916/article/details/40541287](http://blog.csdn.net/sk719887916/article/details/40541287)），怀疑是子控件父控件抢占焦点的问题，所以把 descendantFocusability 可能的三个属性试了个遍，都没好使。
* 最后定位问题出在加监听的位置不对，应该用 onListItemClick() 方法实现点击监听，这是 ListActivity 的方法，我们理应这么做。之前的做法属于野路子。