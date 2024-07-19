    //chart1 setup
    const data = {
     datasets: [{
        label: 'spo2',
        backgroundColor: '#084de0',
        borderColor:`#084de0`,

        borderWidth: 2
      }]
    };
    //chart1 config
        // config 
        const config = {
      type: 'line',
      data,
      options: {
        scales: {
          y: {
            beginAtZero: true,
            title: {
                display: true,
                text: 'spo2'
              }
          },
          x: {
            beginAtZero: true,
            title: {
                display: true,
                text: 'time'
              }
          }
        }
      }
    };

        //chart1 setup
        const dataB = {
            datasets: [{
               label: 'heartrate',
               backgroundColor: '#FF0000',
               borderColor: '#FF0000',
               borderWidth: 2
             }]
           };
           //chart1 config
               // config 
               const configB = {
             type: 'line',
             data: dataB,
             options: {
                scales: {
                    y: {
                      beginAtZero: true,
                      title: {
                          display: true,
                          text: 'heartRate'
                        }
                    },
                    x: {
                      beginAtZero: true,
                      title: {
                          display: true,
                          text: 'time'
                        }
                    }
                  }
             }
           };
    
           
    const myChart = new Chart(
      document.getElementById('myChart'),
      config
    );
    const myChart2 = new Chart(
        document.getElementById('myChart2'),
        configB
      );
    function updateChart(){
        async function fetchData(){
        let url = 'http://domain/jsonfile.php';
        let promiseResponse = await fetch(url);
        let dataArray = await promiseResponse.json();
        console.log(dataArray);
        return dataArray;
    };
    fetchData().then( dataArray => {
         let spo2Array = dataArray.map( (val,index) => {
            return dataArray[index].spo2;
        });
        
        let heartRateArray = dataArray.map( (value, index ) =>{
            return dataArray[index].heartRate;
        })
        
       let timeStampArray = dataArray.map( (value, index) =>{
        return dataArray[index].timestamp;
       })
        console.log(`spo2`,spo2Array);
        console.log(`time`,timeStampArray);
        console.log(heartRateArray);
        //myChart.config.data.labels = month;
        //myChart.config.data.datasets[0].data = value;
        if(myChart.config.data.labels.length>12){
            myChart.config.data.labels.shift();
            myChart.config.data.datasets[0].data.shift();
        }

        myChart.config.data.labels.push(timeStampArray[0]);
        myChart.config.data.datasets[0].data.push(spo2Array[0]);

        if(myChart2.config.data.labels.length>12){
            myChart2.config.data.labels.shift();
            myChart2.config.data.datasets[0].data.shift();
        }
        
        myChart2.config.data.labels.push(timeStampArray[0]);
        myChart2.config.data.datasets[0].data.push(heartRateArray[0]);
        myChart.update();
        myChart2.update();
    })
}
setInterval(updateChart,1000);
