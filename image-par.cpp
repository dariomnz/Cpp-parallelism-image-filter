#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <string.h>
#include <chrono>
#include <omp.h>

using std::vector;
using namespace std::chrono;
using clk = high_resolution_clock;

// Alinea los datos a una palabra
#pragma pack(1)

// Definicion de nuestros tamaños de palabra
// tamaño 4 bytes con valores negativos
typedef int LONG;
// tamaño 2 bytes
typedef unsigned short WORD;
// tamaño 4 bytes sin valores negativos
typedef unsigned int DWORD;


//estructura de datos de la cabecera bmp 1
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

// Estructura de datos de la cabecera bmp 2
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

// Estructura utilizada para guardar los datos de cada archivo bmp
typedef struct tagfile
{
    std::string file_name;
    microseconds read_time;
    microseconds write_time;
    microseconds gauss_time;
    microseconds sobel_time;
    PBITMAPFILEHEADER self_fileheader;
    PBITMAPINFOHEADER selg_infoheader;
    char* filedata;

}file;

//Estructura para guardar los datos de cada pixel
struct pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

//metodo para guardar las fotos en un vector de files
static vector<file> get_files(char * dir_name){
    //creamos el vector a devolver
    vector<file> files;
    //preparamos el directorio
    DIR *dir;
    dir = opendir(dir_name);
    dirent *entrada;
    file* new_file;
    //bucle para leer los archivos del directorio
    while((entrada = readdir(dir)) != NULL )
    {
        //comprobamos es del tipo que queremos
        if (entrada->d_type == DT_REG)
        {   
            //variable calculo del tiempo de load
            auto t1 = clk :: now();
            //variables para la ruta
            std::string separator = "/";
            std::string file_path = dir_name + separator + entrada->d_name;
            //abrir el archivo
            std::ifstream _file(file_path);
            //si existe
            if (_file) {
                _file.seekg(0, std::ios::end);
                std::streampos length = _file.tellg();
                _file.seekg(0, std::ios::beg);
                //leemos el archivo
                char* buffer = new char[length];
                _file.read(&buffer[0], length);
                //creamos las variables para guardar los datos
                PBITMAPFILEHEADER file_header;
                PBITMAPINFOHEADER info_header;
                //recogemos los datos cogiendo posiciones de puntero
                file_header = (PBITMAPFILEHEADER) (&buffer[0]);
                info_header = (PBITMAPINFOHEADER) (&buffer[0] + sizeof(BITMAPFILEHEADER));
                //creamos el archivo para guardar la foto y guardamos los datos obtenidos
                new_file = new file;
                new_file->filedata = buffer;
                new_file->file_name = entrada->d_name;
                new_file->self_fileheader = file_header;
                new_file->selg_infoheader = info_header;

                //comprobamos que los atributos son los de una foto BMP
                if (new_file->self_fileheader->bfType != 0x4D42) {

                    fprintf(stderr,"File %s is not a .bmp!\n",file_path.c_str());  
                }else if (new_file->selg_infoheader->biPlanes != 1) {

                    fprintf(stderr,"File %s has not 1 plane!\n",file_path.c_str()); 
                }else if (new_file->selg_infoheader->biBitCount != 24) {
                
                    fprintf(stderr,"File %s has not 24 bit per pixel!\n",file_path.c_str());
                }else if (new_file->selg_infoheader->biCompression != 0) {
                    
                    fprintf(stderr,"File %s has not 0 compresion!\n",file_path.c_str());
                }else {
                    //calculo de tiempo de load
                    auto t2 = clk :: now();
                    auto diff  = duration_cast<microseconds>(t2-t1);
                    new_file->read_time = diff;
                    //guardamos el archivo
                    files.push_back(*new_file);
                }
            }
            //mensaje de error de que el archivo no existe
            else {
                fprintf(stderr,"File %s don't Exist!\n",file_path.c_str());
            }
        }
    }
    //cerramos el directorio y devolvemos las fotos obtenidas
    closedir(dir);
    return files;
}


//metodo de aplicacion de difusion gaussiana
static void gauss(pixel **& pixels,file* file){
    //atributos de cada fotografia
    int height = file->selg_infoheader->biHeight;
    int width = file->selg_infoheader->biWidth;
    //creamos una matriz de pixeles donde guardar los pixeles de las fotografias, y reservamos memoria
    pixel** pixels_aux = new pixel*[height];
    for (int i = 0; i < height; i++)
        pixels_aux[i] = new pixel[width];
    //peso
    float weight=1/273.0f;
    //matriz de gauss
    int m[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}}; // matriz m de calculo

    omp_set_num_threads(4);
    #pragma omp parallel for 
    // #pragma omp parallel for schedule(static) 
    // #pragma omp parallel for schedule(dynamic) 
    // #pragma omp parallel for schedule(guided) 

    // bucle del calculo
    //bucles para recorrer la foto y obtener el pixel
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            //reiniciamos el resultado de cada color a 0 antes del calculo
            int result_r = 0;
            int result_g = 0;
            int result_b = 0;
            //bucles para el sumatorio y recorrer la matriz de gauss
            for (int s = -2; s <= 2; s++)
            {
                //si el valor de s provoca un valor prohibido, el resultado es 0 y no se ejecuta el resto
                if ((i+s) < 0 || (i+s) >= height)
                {
                    result_r += 0;
                    result_g += 0;
                    result_b += 0;
                    
                }else
                {
                    for (int t = -2; t <= 2; t++)
                    {
                        //si j provoca un valor prohibido, se salta su ejecucion y se suma 0
                        if ((j+t) < 0 || (j+t) >= width)
                        {
                            result_r += 0;
                            result_g += 0;
                            result_b += 0;
                        }else
                        {
                            //si s y t son validos se realiza la funcion de gauss sobre cada color y se suma a los anteriores
                            result_r += m[s+2][t+2]*pixels[i+s][j+t].r;
                            result_g += m[s+2][t+2]*pixels[i+s][j+t].g;
                            result_b += m[s+2][t+2]*pixels[i+s][j+t].b;
                        }
                    }
                }
            } 
            //se guarda el resultado del sumatorio
            pixels_aux[i][j].r = weight*result_r;
            pixels_aux[i][j].g = weight*result_g;
            pixels_aux[i][j].b = weight*result_b;
        }
    }
    //igualamos pixel a pixel_aux para tener en pixel el valor calculado
    pixels = pixels_aux;
}

//metodo para aplicar el filtro sobel
static void sobel(pixel **& pixels,file* file){
    //propiedades de las fotografias
    int height = file->selg_infoheader->biHeight;
    int width = file->selg_infoheader->biWidth;
    //creamos una matriz auxiliar de pixeles donde guardar los pixeles de las fotografias, y reservamos memoria
    pixel** pixels_aux = new pixel*[height];
    for (int i = 0; i < height; i++)
        pixels_aux[i] = new pixel[width];

    float weight=1/8.0f;

    int m_x[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}}; // matriz m_x de calculo
    int m_y[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}}; // matriz m_y de calculo

    // bulce de de calculo
    omp_set_num_threads(4);
    #pragma omp parallel for 
    // #pragma omp parallel for schedule(static) 
    // #pragma omp parallel for schedule(dynamic) 
    // #pragma omp parallel for schedule(guided) 

    // bucle de de calculo
    //recorrer la matriz de pixeles
    for (int i = 0; i < height; i++)
    {   
        for (int j = 0; j < width; j++)
        {
            //variables para los filtros
            int result_x_r=0;
            int result_x_g=0;
            int result_x_b=0;
            int result_y_r=0;
            int result_y_g=0;
            int result_y_b=0;
            //sumatorios de la funcion sobel
            for (int s = -1; s <= 1; s++)
            {
                //si s da un valor prohibido, se suma 0 en vez de hacer la operacion
                if ((i+s) < 0 || (i+s) >= height)
                {
                    result_x_r+=0;
                    result_x_g+=0;
                    result_x_b+=0;
                    result_y_r+=0;
                    result_y_g+=0;
                    result_y_b+=0;
                    
                }else
                {
                    //si t da un valor prohibido, se suma 0 en vez de hacer la operacion
                    for (int t = -1; t <= 1; t++)
                    {
                        if ((j+t) < 0 || (j+t) >= width)
                        {
                            result_x_r+=0;
                            result_x_g+=0;
                            result_x_b+=0;
                            result_y_r+=0;
                            result_y_g+=0;
                            result_y_b+=0;
                        }else
                        {
                            //las operaciones de sobel
                            result_x_r += m_x[s+1][t+1]*pixels[i+s][j+t].r;
                            result_x_g += m_x[s+1][t+1]*pixels[i+s][j+t].g;
                            result_x_b += m_x[s+1][t+1]*pixels[i+s][j+t].b;

                            result_y_r +=m_y[s+1][t+1]*pixels[i+s][j+t].r;
                            result_y_g +=m_y[s+1][t+1]*pixels[i+s][j+t].g;
                            result_y_b +=m_y[s+1][t+1]*pixels[i+s][j+t].b;

                        }
                    }
                }
            }
            //el valor obtenido de los sumatorios se guardan en la posicion indicada
            pixels_aux[i][j].r = abs(weight*result_x_r) + abs(weight*result_y_r);
            pixels_aux[i][j].g = abs(weight*result_x_g) + abs(weight*result_y_g);
            pixels_aux[i][j].b = abs(weight*result_x_b) + abs(weight*result_y_b);
        }
    }
    //guardamos en pixels los valores obtenidos
    pixels = pixels_aux;
}
//metodo para obtener los pixeles de las fotos en formato BMP24
static pixel** GetPixlesFromBMP24(file* file) {
    //propiedades de las fotografias
    int rows = file->selg_infoheader->biHeight;
    int cols = file->selg_infoheader->biWidth;
    //creamos una matriz de pixeles donde guardar los pixeles de las fotografias, y reservamos memoria
    pixel** pixels = new pixel*[rows];
    for (int i = 0; i < rows; i++)
        pixels[i] = new pixel[cols];
    //contador para el bucle
    int count = 1;
    int extra = cols % 4; // El numero de bytes en una fila (columna) si es multiplo de 4.
    //recorrer la matriz por filas
    for (int i = 0; i < rows; i++){
        //sumo al contador los valores sobrantes si no es multiplo de 4
        count += extra;
        //recorrer las columnas
        for (int j = cols - 1; j >= 0; j--) {
            //obtener los 3 colores, RGB
            for (int k = 0; k < 3; k++) {
                switch (k) {
                //obtener el valor rojo
                case 0:
                    pixels[i][j].r = file->filedata[file->self_fileheader->bfSize - count++];
                    break;
                //obtener el valor verde
                case 1:
                    pixels[i][j].g = file->filedata[file->self_fileheader->bfSize - count++];
                    break;
                //obtener el valor azul
                case 2:
                    pixels[i][j].b = file->filedata[file->self_fileheader->bfSize - count++];
                    break;
                }
            }
        }
    }
    //devuelvo la matriz obtendia
    return pixels;
}
//metodo para escribir en las fotografias los pixeles, y guardarlas en el directorio
static void WriteOutBmp24(file* file,char* dir_name,pixel** pixels) {
    //valores de cada fotografia
    int rows = file->selg_infoheader->biHeight;
    int cols = file->selg_infoheader->biWidth;
    //variables para crear los nombres de los directorios
    std::string separator = "/";
    std::string file_path = dir_name + separator +file->file_name;
    //escrbir en el camino indicado
    std::ofstream write(file_path);
    //comprobar que hemos logrado escribir
    if (!write) {
        fprintf(stderr,"Failed to write [%s]\n",file_path.c_str());  
        exit(-1);
    }
    int count = 1;
    int extra = cols % 4; // El numero de bytes de cada fila (columna) debe de ser 4.
    for (int i = 0; i < rows; i++){

        count += extra;
        for (int j = cols - 1; j >= 0; j--) {
            //guardar cada bloque de colores
            for (int k = 0; k < 3; k++) {
                switch (k) {
                case 0: //rojo
                    file->filedata[file->self_fileheader->bfSize - count] = pixels[i][j].r;
                    break;
                case 1: //verde
                    file->filedata[file->self_fileheader->bfSize - count] = pixels[i][j].g;
                    break;
                case 2: //azul
                    file->filedata[file->self_fileheader->bfSize - count] = pixels[i][j].b;
                    break;
                }
                count++;
            }
        }
    }
    //escribir en el lugar indicado
    write.write(file->filedata, file->self_fileheader->bfSize);
}
//metodo para escribir en las fotografias en el directorio
static void WriteOut(file* file,char* dir_name) {
    //variables para crear la ruta
    std::string separator = "/";
    std::string file_path = dir_name + separator + file->file_name;
    //escribir en la ruta indicada
    std::ofstream write(file_path);
    //si no hemos escrito mandar mensaje de error
    if (!write) {
        fprintf(stderr,"Failed to write [%s]\n",file_path.c_str()); 
    }
    
    write.write(file->filedata, file->self_fileheader->bfSize);
}
//comprobar que el directorio existe
static bool check_directory(char * dir_name){
    DIR *dir;
    //abrimos el directorio
    dir = opendir(dir_name);
    //si no existe, devolvemos falso
    if (dir == NULL)
    {
        return false;
    }
    //cerramos el directorio
    closedir(dir);
    //como existe, devolvemos verdadero
    return true;
}

int main(int argc, char** argv) {
   //comprobar que el numero de argumentos es el correcto
    if (argc != 4)
    {
       fprintf(stderr,"Wrong format:\n image-seq operation in_path out_path\n operation: copy, gauss, sobel\n");
       exit(-1);
    }
    //imprimir el nombre de los directorios dados
    printf("Input path: %s\n",argv[2]);
    printf("Output path: %s\n",argv[3]);
    //comprobar directorio de entrada existe
    if(!check_directory(argv[2]))
    { 
        fprintf(stderr,"Cannot open directory [%s]\n\t image-seq operation in_path out_path\n\t\t operation: copy, gauss, sobel\n",argv[2]); 
        exit(-1);
    }
    //comprobar que el directorio de salida existe
    if(!check_directory(argv[3]))
    {
        fprintf(stderr,"Output directory [%s] does not exist\n\t image-seq operation in_path out_path\n\t\t operation: copy, gauss, sobel\n",argv[3]);  
        exit(-1);
    }
    //comprobar que el comando es uno de los tres permitidos
    if (!(strcmp(argv[1],"copy") == 0 || strcmp(argv[1],"gauss") == 0 || strcmp(argv[1],"sobel") == 0)){
        fprintf(stderr,"Unexpected operation: %s\n\timage-seq operation in_path out_path\n\t\toperation: copy, gauss, sobel\n",argv[1]);
        exit(-1);
    }

    //creamos un vector con todas las fotos
    vector<file> files = get_files(argv[2]);    
    //vemos cual es el comando pedido y llamamos a su funcion
    //si el comando es copy
    if(strcmp(argv[1],"copy") == 0)
    {
        for (auto &&file : files){
            //tiempo antes de copiar
            auto t1 = clk::now();
            //llamamos a la funcion que copia las fotos en el directorio indicado de destino
            WriteOut(&file, argv[3]);
            //tiempo despues
            auto t2 = clk :: now();
            //tiempo de copia
            auto diff  = duration_cast<microseconds>(t2-t1);
            file.write_time = diff;
        }
        //comprobar si el comando es gauss
    }else if (strcmp(argv[1],"gauss") == 0)
    {
        for (auto &&file : files){
            auto t1 = clk::now();
            //guardar los pixeles de cada foto en una matriz para acceder a ellos facilmente y pasarselo a gauss
            pixel** pixels = GetPixlesFromBMP24(&file);
            gauss(pixels, &file);

            auto t2 = clk :: now();
            //calculo de tiempo de ejecucion de gauss
            auto diff  = duration_cast<microseconds>(t2-t1);
            file.gauss_time = diff;


            t1 = clk::now();
            //copia de los archivos en el destino
            WriteOutBmp24(&file, argv[3], pixels);

            t2 = clk :: now();
            //calculo de tiempo de copia
            diff  = duration_cast<microseconds>(t2-t1);
            file.write_time = diff;
        }
            //hacer sobel, que es el restante
    }else if (strcmp(argv[1],"sobel") == 0)
    {
        for (auto &&file : files){
            auto t1 = clk::now();
            //guardar los pixeles de cada foto en una matriz para acceder a ellos facilmente y pasarselo a gauss
            pixel** pixels = GetPixlesFromBMP24(&file);
            //llamada a gauss
            gauss(pixels, &file);

            auto t2 = clk :: now();
            //calculo de tiempo de realizacion de gauss
            auto diff  = duration_cast<microseconds>(t2-t1);
            file.gauss_time = diff;

            t1 = clk::now();
            //con los pixeles usados en gauss, llamamos a sobel
            sobel(pixels, &file);

            t2 = clk :: now();
            //tiempo en ejecutar sobel
            diff  = duration_cast<microseconds>(t2-t1);
            file.sobel_time = diff;


            t1 = clk::now();
            //copia de los archivos en el directorio de destino
            WriteOutBmp24(&file, argv[3], pixels);

            t2 = clk :: now();
            //calculo del tiempo de copia
            diff  = duration_cast<microseconds>(t2-t1);
            file.write_time = diff;
        }  
    }

    // Imprimir los valores de cada tiempo que conlleva cada metodo
    for (auto &&file : files){

        auto total_time = file.read_time + file.write_time + file.gauss_time + file.sobel_time;
        //tiempo hemos tardado en procesar esa foto concreta
        printf("File: \"%s\"(time: %ld)\n",file.file_name.c_str(),total_time.count());
        //tiempo hemos tardado en cargar las fotos
        printf("   Load time: %ld\n",file.read_time.count());
        //si se ha hecho gauss, imprimir su tiempo
        if (file.gauss_time.count() > 0)
        {
            printf("   Gauss time: %ld\n",file.gauss_time.count());
        }
        //si se ha hecho sobel, imprimir su tiempo
        if (file.sobel_time.count() > 0)
        {
            printf("   Sobel time: %ld\n",file.sobel_time.count());
        }
        //tiempo que hemos tardando en copiarlo en directorio de destino
        printf("   Store time: %ld\n",file.write_time.count());
    }
    
}