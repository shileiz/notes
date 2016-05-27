#coding:utf8

import markdown
import os.path
import os

md_list = [f for f in os.listdir(os.getcwd()) if f.endswith('.md')]

for md in md_list:
    out_file = md.replace(".md", ".html")
    if os.path.exists(os.path.join(os.getcwd(),out_file)):
        os.remove(out_file)
    with open(out_file, 'a') as output_html:
        with open("template_header.txt","r") as template_header:
            output_html.write(template_header.read())
        with open(md,"r") as input_md:
            md = markdown.Markdown(output_format="html4")
            md.convertFile(input=input_md, output=output_html)
        with open("template_footer.txt","r") as template_footer:
            output_html.write(template_footer.read())        
    
        

