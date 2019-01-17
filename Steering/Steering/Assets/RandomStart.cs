using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RandomStart : MonoBehaviour {

	// Use this for initialization
	void Start () {
        transform.position = new Vector3(Random.value * 50, 1.0f, Random.value * 50);
	}
	
	// Update is called once per frame
	void Update () {
		
	}
}
