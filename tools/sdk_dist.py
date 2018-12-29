import os
import sys
cwd_path = os.getcwd()
sys.path.append(os.path.join(os.path.dirname(cwd_path), 'rt-thread', 'tools'))

def bsp_update_driver_kconfig(dist_dir):
    # change RTT_ROOT in Kconfig
    if not os.path.isfile(os.path.join(dist_dir, 'Kconfig')):
        return

    with open(os.path.join(dist_dir, 'Kconfig'), 'r') as f:
        data = f.readlines()
    with open(os.path.join(dist_dir, 'Kconfig'), 'w') as f:
        found = 0
        for line in data:
            if line.find('RTT_ROOT') != -1:
                found = 1
            if line.find('source "$BSP_DIR/../../drivers') != -1 and found:
                position = line.find('source "$BSP_DIR/../../drivers')
                line = line[0:position] + 'source "$BSP_DIR/drivers/Kconfig"\n'
                found = 0
            f.write(line)

def bsp_update_library_kconfig(dist_dir):
    # change RTT_ROOT in Kconfig
    if not os.path.isfile(os.path.join(dist_dir, 'Kconfig')):
        return

    with open(os.path.join(dist_dir, 'Kconfig'), 'r') as f:
        data = f.readlines()
    with open(os.path.join(dist_dir, 'Kconfig'), 'w') as f:
        found = 0
        for line in data:
            if line.find('RTT_ROOT') != -1:
                found = 1
            if line.find('source "$BSP_DIR/../../libraries') != -1 and found:
                position = line.find('source "$BSP_DIR/../../libraries')
                line = line[0:position] + 'source "$BSP_DIR/libraries/Kconfig"\n'
                found = 0
            f.write(line)   

# BSP dist function
def dist_do_building(BSP_ROOT):
    from mkdist import bsp_copy_files
    
    dist_dir  = os.path.join(BSP_ROOT, 'dist', os.path.basename(BSP_ROOT))
    print("=> copy bsp library")
    library_path = os.path.join(os.path.dirname(os.path.dirname(BSP_ROOT)), 'libraries')
    library_dir  = os.path.join(dist_dir, 'libraries')
    bsp_copy_files(library_path, library_dir)
    
    print("=> copy bsp drivers")
    driver_path  = os.path.join(os.path.dirname(os.path.dirname(BSP_ROOT)), 'drivers')
    driver_dir   = os.path.join(dist_dir, 'drivers')
    bsp_copy_files(driver_path, driver_dir)
    
    iar_template_file =  os.path.join(dist_dir, 'template.ewp')
    try:
        if os.path.isfile(iar_template_file):
            with open(iar_template_file, 'r+') as f:
                file_content = f.read()

            file_content = file_content.replace('$PROJ_DIR$\..\..\drivers\linker_scripts', '$PROJ_DIR$\drivers\linker_scripts')
            
            with open(iar_template_file, 'w') as f:
                f.write(str(file_content))
    except Exception, e:
        print('e.message:%s\t' % e.message)
        
    bsp_update_driver_kconfig(dist_dir)
    bsp_update_library_kconfig(dist_dir)
