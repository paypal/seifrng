#include<iostream>
int main(){
    std::cout<<std::endl;
    for (uint8_t i = 0;; ++i){
        uint8_t thisByte = i;
        uint8_t c;
        for (c = 0; thisByte; thisByte>>=1){
            c += thisByte & 1; 
        }
        //std::cout<<(uint)i<<" "<<(uint)c<<" "<<(float)c/(float)8<<std::endl;
        std::cout<<static_cast<float>(c)/static_cast<float>(8)<<",";
        if(i==255)
            break;
    }
    std::cout<<std::endl;
}
