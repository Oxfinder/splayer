#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/soundcard.h>
#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>
#endif
#define AUDIO_DEVICE "/dev/dsp"

#define ALSA_MAX_BUF_SIZE 655350

#ifdef HAVE_ALSA
int play_sound_Alsa(char* filename,int rate,int bits,int channel,int order)
{
	int rc,size;
	char *buffer;
	unsigned int val;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames,periodsize;
	snd_mixer_t *mixer;
	snd_mixer_elem_t *pcm_element;
	struct stat buf;
	
	if(stat(filename, &buf)<0)
	{
		return -1;
	}
	size=(unsigned long)buf.st_size;

	FILE *fp = fopen(filename,"rb");
	if(NULL == fp)
	{
		printf("can't open file!\n");
		return -1;
	}
	
	rc = snd_pcm_open(&handle,"default",SND_PCM_STREAM_PLAYBACK,0);

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle,params);
	snd_pcm_hw_params_set_access(handle,params,SND_PCM_ACCESS_RW_INTERLEAVED);
	switch(order){
		case 1:
			snd_pcm_hw_params_set_format(handle,params,SND_PCM_FORMAT_S16_LE);
			break;
		case 2:
			snd_pcm_hw_params_set_format(handle,params,SND_PCM_FORMAT_S16_BE);
			break;
		defualt:
			break;
	}
	snd_pcm_hw_params_set_channels(handle,params,channel);

	val = rate;
	snd_pcm_hw_params_set_rate_near(handle,params,&val,0);
	snd_pcm_hw_params_get_buffer_size_max(params,&frames);
	frames = frames <  ALSA_MAX_BUF_SIZE? frames:ALSA_MAX_BUF_SIZE;
	rc = snd_pcm_hw_params_set_buffer_size_near(handle,params,&frames);
	snd_pcm_hw_params_get_period_size_min(params,&periodsize,NULL);
	if(!periodsize){
		periodsize=size/4;
	}
	rc = snd_pcm_hw_params_set_period_size_near(handle,params,&periodsize,NULL);
	rc = snd_pcm_hw_params(handle,params);

	snd_mixer_open(&mixer,0);	
	snd_mixer_attach(mixer,"default");
	snd_mixer_selem_register(mixer,NULL,NULL);
	snd_mixer_load(mixer);
	for(pcm_element = snd_mixer_first_elem(mixer);pcm_element;pcm_element=snd_mixer_elem_next(pcm_element))
	{
		if(snd_mixer_elem_get_type(pcm_element)==SND_MIXER_ELEM_SIMPLE && snd_mixer_selem_is_active(pcm_element))
		{
			if(!strcmp(snd_mixer_selem_get_name(pcm_element),"Master"))
			{
				snd_mixer_selem_set_playback_volume_range(pcm_element,0,100);
				snd_mixer_selem_set_playback_volume_all(pcm_element,(long)100);
			}
		}
	}
	buffer = (char*)malloc(size);
	rc = fread(buffer,1,size,fp);
	if(0== rc)
		return -1;
	snd_pcm_writei(handle,buffer,size);

	snd_pcm_prepare(handle);
	snd_pcm_hw_params_free(params);
	snd_mixer_close(mixer);
	snd_pcm_close(handle);
	free(buffer);
	fclose(fp);
	return 0;
}
#endif

int play_sound_OSS(char *filename,int rate,int bits){
        struct stat stat_buf;
        unsigned char *buf = NULL;
        int result,arg,status,handler,fd;

        fd = open(filename,O_RDONLY);
        if(fd<0)
        return -1;

        if(fstat(fd,&stat_buf))
        {
                close(fd);
                return -1;
        }

        if(!stat_buf.st_size)
        {
                close(fd);
                return -1;
        }

        buf=malloc(stat_buf.st_size);
        if(!buf){
                close(fd);
                return -1;
        }

        if(read(fd,buf,stat_buf.st_size)<0){
                free(buf);
                close(fd);
                return -1;
        }

        handler = open(AUDIO_DEVICE,O_WRONLY);
        if(-1 == handler){
                return -1;
        }

        arg = rate*2;
        status = ioctl(handler,SOUND_PCM_WRITE_RATE,&arg);
        if(-1 == status)
                return -1;

        arg = bits;
        status = ioctl(handler,SOUND_PCM_WRITE_BITS,&arg);
        if(-1 == status)
                return -1;

        result = write(handler,buf,stat_buf.st_size);
        if(-1 == result)
                return -1;

        free(buf);
        close(fd);
        close(handler);
        return result;

}

/*中文显示乱码问题*/
int main(int argc,char** argv){
	if(*argv[4] == '1')
        	play_sound_OSS(argv[1],atoi(argv[2]),atoi(argv[3]));
#ifdef HAVE_ALSA
	else
		play_sound_Alsa(argv[1],atoi(argv[2]),atoi(argv[3]),atoi(argv[5]),atoi(argv[6]));
#endif
        return 0;
}
